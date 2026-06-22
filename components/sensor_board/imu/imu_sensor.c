#include "imu_sensor.h"
#include "logging.h"
#include "result.h"
#include "stm32h7xx_hal.h"
#include <math.h>
#include <string.h>

void imu_sensor_init(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to init");
        return;
    }
    
    LOGI("IMU", "Initializing IMU sensor");
    
    // Initializing sensor data to zero
    memset(imu, 0, sizeof(imu_data_t));
}

void imu_sensor_read(imu_data_t *imu, imu_data_t *data) {
    if (imu == NULL || data == NULL) {
        LOGE("IMU", "NULL pointer provided");
        return;
    }
    *data = *imu;     // Copying the current IMU data
}

result_t imu_sensor_update(imu_data_t *imu, float ax, float ay, float az,
                        float gx, float gy, float gz,
                        float mx, float my, float mz,
                        uint32_t timestamp) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to update");
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Updating accelerometer data (m/s²)
    imu->accel[0] = ax;
    imu->accel[1] = ay;
    imu->accel[2] = az;

    // Updating gyroscope data (degrees/second) (ANGULAR VELOCITY)
    imu->gyro[0] = gx;
    imu->gyro[1] = gy;
    imu->gyro[2] = gz;

    // Updating magnetometer data (in µT)
    imu->mag[0] = mx;
    imu->mag[1] = my;
    imu->mag[2] = mz;

    // Updating the timestamp
    imu->timestamp = timestamp;
    imu->last_timestamp = timestamp;

    return RESULT_OK;
}

/* ===========================================================================
 * Xsens Avior IMU — I2C (Xbus) driver
 * ===========================================================================
 * The Xsens Avior speaks the Xbus protocol. Over I2C (Xsens MTi-1-series
 * compatible interface) Xbus messages move through "pipe" opcodes that are
 * used as an 8-bit register address:
 *   0x03 ControlPipe      — write Xbus command messages here
 *   0x04 PipeStatus       — read 4 bytes: notif size (LE16), meas size (LE16)
 *   0x05 NotificationPipe — read a pending notification (command ack) message
 *   0x06 MeasurementPipe  — read a pending MTData2 measurement message
 *
 * Xbus message layout (what we write to / read from a pipe):
 *   [0xFA][0xFF][MID][LEN][DATA…][CHK]
 *   CHK makes (BID + MID + LEN + DATA + CHK) & 0xFF == 0  (preamble excluded)
 *
 * IMPORTANT: the pipe opcodes, the default 7-bit I2C address (0x6B) and the
 * Xbus data identifiers below are the documented Xsens MTi-1-series values.
 * The Avior is Xbus-compatible but CONFIRM these against the Avior datasheet
 * (mtidocs.xsens.com) and override with -D flags if your unit differs.
 * ========================================================================= */

extern I2C_HandleTypeDef hi2c1;
#ifndef XSENS_I2C_HANDLE
#define XSENS_I2C_HANDLE hi2c1
#endif

#ifndef XSENS_I2C_ADDR_7BIT
#define XSENS_I2C_ADDR_7BIT 0x6BU /* MTi-1 series default */
#endif
#define XSENS_I2C_ADDR_8BIT (XSENS_I2C_ADDR_7BIT << 1)

#ifndef XSENS_I2C_TIMEOUT_MS
#define XSENS_I2C_TIMEOUT_MS 100U
#endif

/* Output data rate requested for accel/gyro/mag (Hz) */
#ifndef XSENS_OUTPUT_RATE_HZ
#define XSENS_OUTPUT_RATE_HZ 100U
#endif

/* Pipe opcodes (used as an 8-bit "register" address) */
#define XSENS_OPCODE_CONTROL 0x03U
#define XSENS_OPCODE_STATUS 0x04U
#define XSENS_OPCODE_MEAS 0x06U

/* Xbus framing */
#define XBUS_PREAMBLE 0xFAU
#define XBUS_BID 0xFFU

/* Message IDs */
#define XMID_GOTOCONFIG 0x30U
#define XMID_GOTOMEAS 0x10U
#define XMID_SETOUTPUTCFG 0xC0U
#define XMID_MTDATA2 0x36U

/* MTData2 data identifiers. The low 2 bits select precision (0 = float32),
 * bits 2-3 the coordinate system (ENU/NED/NWU) — we mask those out (& 0xFFF3)
 * and request float32, so only the data type + precision are matched. */
#define XDI_MASK 0xFFF3U
#define XDI_ACCELERATION 0x4020U
#define XDI_RATEOFTURN 0x8020U
#define XDI_MAGNETICFIELD 0xC020U

#define RAD_TO_DEG 57.29577951308232f

static bool s_imu_configured = false;

/* Xbus checksum over [BID, MID, LEN, DATA...] -> the CHK byte that zeroes the
 * 8-bit sum (preamble 0xFA excluded). */
static uint8_t xbus_checksum(const uint8_t *from_bid, uint16_t len) {
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum = (uint8_t)(sum + from_bid[i]);
    }
    return (uint8_t)(0x100U - sum);
}

/* Build + send an Xbus command message to the ControlPipe (opcode 0x03). */
static result_t xbus_write(uint8_t mid, const uint8_t *data, uint8_t len) {
    uint8_t msg[5 + 255];
    msg[0] = XBUS_PREAMBLE;
    msg[1] = XBUS_BID;
    msg[2] = mid;
    msg[3] = len;
    if (len > 0 && data != NULL) {
        memcpy(&msg[4], data, len);
    }
    /* checksum spans BID..last data byte = indices 1..(3+len) => 3+len bytes */
    msg[4 + len] = xbus_checksum(&msg[1], (uint16_t)(3 + len));

    if (HAL_I2C_Mem_Write(&XSENS_I2C_HANDLE, XSENS_I2C_ADDR_8BIT,
                          XSENS_OPCODE_CONTROL, I2C_MEMADD_SIZE_8BIT, msg,
                          (uint16_t)(5 + len), XSENS_I2C_TIMEOUT_MS) != HAL_OK) {
        return RESULT_ERR_COMMS;
    }
    return RESULT_OK;
}

/* Read the number of measurement bytes pending from PipeStatus (opcode 0x04).
 * Status bytes: [notif LE16][meas LE16]. */
static result_t xbus_meas_size(uint16_t *meas_size) {
    uint8_t st[4];
    if (HAL_I2C_Mem_Read(&XSENS_I2C_HANDLE, XSENS_I2C_ADDR_8BIT,
                         XSENS_OPCODE_STATUS, I2C_MEMADD_SIZE_8BIT, st,
                         sizeof(st), XSENS_I2C_TIMEOUT_MS) != HAL_OK) {
        return RESULT_ERR_COMMS;
    }
    *meas_size = (uint16_t)(st[2] | ((uint16_t)st[3] << 8));
    return RESULT_OK;
}

/* Decode a big-endian IEEE-754 float (Xsens is big-endian, STM32 little). */
static float xbus_be_f32(const uint8_t *p) {
    uint32_t u = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
                 ((uint32_t)p[2] << 8) | (uint32_t)p[3];
    float f;
    memcpy(&f, &u, sizeof(f));
    return f;
}

/* One-time device setup: GoToConfig -> SetOutputConfiguration -> GoToMeasure.
 * Small delays let the device process each command (acks drained implicitly via
 * the notification pipe on the next status read). */
static result_t xsens_configure(void) {
    TRY(xbus_write(XMID_GOTOCONFIG, NULL, 0));
    HAL_Delay(50);

    const uint16_t rate = XSENS_OUTPUT_RATE_HZ;
    uint8_t cfg[] = {
        (XDI_ACCELERATION >> 8) & 0xFF,  XDI_ACCELERATION & 0xFF,
        (rate >> 8) & 0xFF,              rate & 0xFF,
        (XDI_RATEOFTURN >> 8) & 0xFF,    XDI_RATEOFTURN & 0xFF,
        (rate >> 8) & 0xFF,              rate & 0xFF,
        (XDI_MAGNETICFIELD >> 8) & 0xFF, XDI_MAGNETICFIELD & 0xFF,
        (rate >> 8) & 0xFF,              rate & 0xFF,
    };
    TRY(xbus_write(XMID_SETOUTPUTCFG, cfg, (uint8_t)sizeof(cfg)));
    HAL_Delay(50);

    TRY(xbus_write(XMID_GOTOMEAS, NULL, 0));
    HAL_Delay(50);
    return RESULT_OK;
}

/* Parse an MTData2 message body, filling whichever of accel/gyro/mag is present.
 * msg points at the full Xbus frame [FA FF 36 LEN ... CHK]. */
static void xsens_parse_mtdata2(const uint8_t *msg, uint16_t msg_len,
                                imu_data_t *imu) {
    if (msg_len < 5 || msg[0] != XBUS_PREAMBLE || msg[1] != XBUS_BID ||
        msg[2] != XMID_MTDATA2) {
        return;
    }
    uint8_t len = msg[3];
    const uint8_t *p = &msg[4];
    const uint8_t *end = p + len;
    if (end > msg + msg_len) {
        end = msg + msg_len; /* guard against a short read */
    }

    float ax = imu->accel[0], ay = imu->accel[1], az = imu->accel[2];
    float gx = imu->gyro[0], gy = imu->gyro[1], gz = imu->gyro[2];
    float mx = imu->mag[0], my = imu->mag[1], mz = imu->mag[2];

    while (p + 3 <= end) {
        uint16_t did = (uint16_t)((p[0] << 8) | p[1]);
        uint8_t dlen = p[2];
        const uint8_t *d = p + 3;
        if (d + dlen > end) {
            break;
        }
        uint16_t kind = did & XDI_MASK;
        if (kind == XDI_ACCELERATION && dlen >= 12) {
            ax = xbus_be_f32(d);          /* m/s^2 */
            ay = xbus_be_f32(d + 4);
            az = xbus_be_f32(d + 8);
        } else if (kind == XDI_RATEOFTURN && dlen >= 12) {
            gx = xbus_be_f32(d) * RAD_TO_DEG;     /* rad/s -> deg/s */
            gy = xbus_be_f32(d + 4) * RAD_TO_DEG;
            gz = xbus_be_f32(d + 8) * RAD_TO_DEG;
        } else if (kind == XDI_MAGNETICFIELD && dlen >= 12) {
            /* Xsens magnetic field is in arbitrary units (~1.0 = local field),
             * NOT micro-tesla. Stored as-is; scale externally if µT needed. */
            mx = xbus_be_f32(d);
            my = xbus_be_f32(d + 4);
            mz = xbus_be_f32(d + 8);
        }
        p = d + dlen;
    }

    imu_sensor_update(imu, ax, ay, az, gx, gy, gz, mx, my, mz, HAL_GetTick());
}

result_t poll_imu_sensor(imu_data_t *imu) {
    if (imu == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (!s_imu_configured) {
        if (xsens_configure() != RESULT_OK) {
            return RESULT_ERR_COMMS; /* device not responding on I2C */
        }
        s_imu_configured = true;
    }

    uint16_t meas_size = 0;
    TRY(xbus_meas_size(&meas_size));
    if (meas_size == 0) {
        return RESULT_ERR_COMMS; /* no fresh sample ready */
    }

    uint8_t buf[128];
    if (meas_size > sizeof(buf)) {
        meas_size = sizeof(buf);
    }
    if (HAL_I2C_Mem_Read(&XSENS_I2C_HANDLE, XSENS_I2C_ADDR_8BIT,
                         XSENS_OPCODE_MEAS, I2C_MEMADD_SIZE_8BIT, buf, meas_size,
                         XSENS_I2C_TIMEOUT_MS) != HAL_OK) {
        return RESULT_ERR_COMMS;
    }

    xsens_parse_mtdata2(buf, meas_size, imu);
    return RESULT_OK;
}

float imu_get_acceleration_magnitude(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_acceleration_magnitude");
        return 0.0f;
    }
    
    return sqrtf(imu->accel[0] * imu->accel[0] + 
                 imu->accel[1] * imu->accel[1] + 
                 imu->accel[2] * imu->accel[2]);
}

// using atan2f because its useful for calculating with floats directly
float imu_get_pitch(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_pitch");
        return 0.0f;
    }
    
    return atan2f(imu->accel[1], 
                  sqrtf(imu->accel[0] * imu->accel[0] + 
                        imu->accel[2] * imu->accel[2])) * 180.0f / M_PI;
}

float imu_get_roll(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_roll");
        return 0.0f;
    }
    
    return atan2f(imu->accel[0],
                  sqrtf(imu->accel[1] * imu->accel[1] +
                        imu->accel[2] * imu->accel[2])) * 180.0f / M_PI;
}

bool imu_validate_accelerometer_range(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to validate_accelerometer_range");
        return false;
    }

    const float max_accel = 16.0f * 9.80665f; // ±16g in m/s²
    
    return (fabsf(imu->accel[0]) <= max_accel &&
            fabsf(imu->accel[1]) <= max_accel &&
            fabsf(imu->accel[2]) <= max_accel);
}

bool imu_validate_gyroscope_range(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to validate_gyroscope_range");
        return false;
    }
    
    const float max_gyro = 2000.0f; // ±2000 deg/s
    
    return (fabsf(imu->gyro[0]) <= max_gyro &&
            fabsf(imu->gyro[1]) <= max_gyro &&
            fabsf(imu->gyro[2]) <= max_gyro);
}

bool imu_validate_magnetometer_range(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to validate_magnetometer_range");
        return false;
    }
    
    const float max_mag = 4900.0f; // ±4900 µT
    
    return (fabsf(imu->mag[0]) <= max_mag &&
            fabsf(imu->mag[1]) <= max_mag &&
            fabsf(imu->mag[2]) <= max_mag);
}

bool imu_check_gyroscope_drift(imu_data_t *imu, float drift_threshold) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to check_gyroscope_drift");
        return false;
    }
    
    return (fabsf(imu->gyro[0]) < drift_threshold &&
            fabsf(imu->gyro[1]) < drift_threshold &&
            fabsf(imu->gyro[2]) < drift_threshold);
}

uint32_t imu_get_timestamp(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_timestamp");
        return 0;
    }
    
    return imu->timestamp;
}






// This takes me back to school in Physics class!

// Mathematical formulas used in this file:
//
// 1. Acceleration Magnitude:
//    |a| = √(ax² + ay² + az²)
//    where ax, ay, az are acceleration components in m/s²
//
// 2. Pitch (rotation around Y-axis):
//    pitch = arctan2(ay, √(ax² + az²)) × (180/π)
//    Converts from radians to degrees
//
// 3. Roll (rotation around X-axis):
//    roll = arctan2(ax, √(ay² + az²)) × (180/π)
//    Converts from radians to degrees
//
// Note: Pitch and Roll calculations assume the device is relatively stationary and use the gravity vector for orientation estimation.