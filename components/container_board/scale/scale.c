#include "scale.h"
#include <logging.h>
#include <result.h>

static char TAG[] = "SCALE";

static float OFFSET = 0; //offset to tare
static float CALIBRATION_FACTOR = 0; //reading/known weight

result_t scale_init(scale_t* scale) {
    scale->isOn = true;
    scale_tare(scale);
    return RESULT_OK;
}

result_t scale_tare(scale_t* scale) {
    if (!scale->isOn) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    OFFSET = scale->raw_data;
    return RESULT_OK;
}

result_t scale_calibrate(scale_t* scale, float tared_weight, float known_weight) {
    if (!scale->isOn || OFFSET == 0) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    LOG(LOG_INFO, TAG, "Make sure you have placed a known weight on the scale");
    CALIBRATION_FACTOR = tared_weight / known_weight;
    return RESULT_OK;
}

result_t scale_read_weight(scale_t* scale, float* res_weight) {
    if (!scale->isOn || OFFSET == 0 || CALIBRATION_FACTOR == 0) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    float weight_tared;
    scale_read_tared(scale, &weight_tared);
    *res_weight =  weight_tared / CALIBRATION_FACTOR;
    return RESULT_OK;
}

result_t scale_turn_off(scale_t* scale) {
    scale->isOn = false;
    OFFSET = 0;
    CALIBRATION_FACTOR = 0;
    return RESULT_OK;
}

result_t scale_read_tared(scale_t* scale, float* res_weight) {
    if (!scale->isOn) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    LOG(LOG_WARNING, TAG, "Note that this will not return an accurate weight since the calibration factor is not accounted for!");
    *res_weight = scale->raw_data - OFFSET;
    return RESULT_OK;
}

