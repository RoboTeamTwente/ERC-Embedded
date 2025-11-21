#ifndef SCALE_H
#define SCALE_H
#include <result.h>
#include <stdbool.h>

typedef struct {
    bool isOn;
    float raw_data;
} scale_t;

/**
 * @brief initializes the scale by turning it on and taring it
 * 
 * @param scale scale struct
 * @return RESULT_OK if OK
 */
result_t scale_init(scale_t* scale);

/**
 * @brief taring the scale sets the OFFSET to the given value
 * 
 * @param scale scale struct
 * @return RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on
 */
result_t scale_tare(scale_t* scale);

/**
 * @brief calibrating the scale retreives the CALIBRATION_FACTOR
 * 
 * @param scale scale struct
 * @param known_weight weight of the calibration weight
 * @return RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on or not tared
 * NOTE: there should be a weight on the scale when doing this
 */
result_t scale_calibrate(scale_t* scale, float tared_weight, float known_weight);

/**
 * @brief reads the accurate weight (desired value)
 * Subtracts the OFFSET and then divides by the CALIBRATION_FACTOR
 * 
 * @param scale scale struct
 * @param weight var to store accurate weight reading
 * @return result_t RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on or not tared or calibrated
 */
result_t scale_read_weight(scale_t* scale, float* res_weight);

/**
 * @brief turns the scale off by also resetting the class vars
 * 
 * @param scale scale struct
 * @return result_t RESULT_OK if OK
 */
result_t scale_turn_off(scale_t* scale);

/**
 * @brief to get a weight read with taring included
 * It subtracts the OFFSET from the raw weight
 * NOTE: the calibration factor is NOT used here
 * 
 * @param scale scale struct
 * @param res_weight var to store the output
 * @return RESULT_OK if OK, RESULT_ERR_NOT_INITIALIZED if the scale is not on
 */
result_t scale_read_tared(scale_t* scale, float* res_weight);

#endif 
