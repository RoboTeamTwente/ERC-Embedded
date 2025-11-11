#ifndef SCALE_H
#define SCALE_H

typedef struct {
    float raw_data;
} scale_t;

/**
 * @brief initializes the scale by calculating the data neccesary for reading accurate weights
 * (The neccessary data being OFFSET and CALIBRATION_FACTOR)
 * 
 * @param scale scale struct
 * NOTE: there should be a weight on the scale when doing this
 */
void init(scale_t* scale, float known_weight);

/**
 * @brief taring the scale sets the OFFSET to the given value
 * 
 * @param scale scale struct
 * NOTE: there should be a weight on the scale when doing this
 */
void tare(scale_t* scale);

/**
 * @brief calibrating the scale retreives the CALIBRATION_FACTOR
 * 
 * @param scale scale struct
 * @param known_weight weight of the calibration weight
 * NOTE: there should be a weight on the scale when doing this
 */
void calibrate(scale_t* scale, float known_weight);

/**
 * @brief reads the accurate weight (desired value)
 * Subtracts the OFFSET and then divides by the CALIBRATION_FACTOR
 * 
 * @param scale scale struct
 * @return float accurate weight reading
 */
float read_weight(scale_t* scale);

/**
 * @brief HELPER FUNCTION to get a weight read with taring included
 * It subtracts the OFFSET from the raw weight
 * NOTE: the calibration factor is NOT used here
 * 
 * @param scale scale struct
 * @return float tared weight
 */
static float read_tared(scale_t* scale);

#endif 
