#ifndef SCALE_H
#define SCALE_H
#include <result.h>
#include <stdbool.h>

typedef struct {
    bool isOn;
    float raw_data;
} scale_t;

/**
 * @brief initializes the scale by calculating the data neccesary for reading accurate weights
 * (The neccessary data being OFFSET and CALIBRATION_FACTOR)
 * 
 * @param scale scale struct
 * @return RESULT_OK if OK
 * NOTE: a weight should be placed on the scale at some point when doing this
 */
result_t init(scale_t* scale, float known_weight);

/**
 * @brief taring the scale sets the OFFSET to the given value
 * 
 * @param scale scale struct
 * @return RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on
 */
result_t tare(scale_t* scale);

/**
 * @brief calibrating the scale retreives the CALIBRATION_FACTOR
 * 
 * @param scale scale struct
 * @param known_weight weight of the calibration weight
 * @return RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on or not tared
 * NOTE: there should be a weight on the scale when doing this
 */
result_t calibrate(scale_t* scale, float known_weight);

/**
 * @brief reads the accurate weight (desired value)
 * Subtracts the OFFSET and then divides by the CALIBRATION_FACTOR
 * 
 * @param scale scale struct
 * @param weight var to store accurate weight reading
 * @return result_t RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on or not tared or calibrated
 */
result_t read_weight(scale_t* scale, float weight);

/**
 * @brief turns off the scale
 * 
 * @param scale scale struct
 * @return result_t RESULT_OK if ok
 */
result_t turnOff(scale_t* scale) {
    scale->isOn = false;
}

/**
 * @brief HELPER FUNCTION to get a weight read with taring included
 * It subtracts the OFFSET from the raw weight
 * NOTE: the calibration factor is NOT used here
 * 
 * @param scale scale struct
 * @param weight_tared var to store the output
 * @return RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on
 */
static result_t read_tared(scale_t* scale, float weight_tared);

#endif 
