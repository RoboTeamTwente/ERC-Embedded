#include "scale.h"
#include <logging.h>
#include <result.h>

static const char TAG[] = "SCALE";

static float OFFSET = 0; //offset to tare
static float CALIBRATION_FACTOR = 0; //reading/known weight

result_t init(scale_t* scale, float known_weight) {
    scale->isOn = true;
    tare(scale);
    calibrate(scale, known_weight);
    return RESULT_OK;
}

result_t tare(scale_t* scale) {
    if (!scale->isOn) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    OFFSET = scale->raw_data;
    return RESULT_OK;
}

result_t calibrate(scale_t* scale, float known_weight) {
    if (!scale->isOn || OFFSET == 0) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    LOG(LOG_INFO, TAG, "Make sure you have placed a known weight on the scale");
    float weight_tared;
    read_tared(scale, weight_tared);
    CALIBRATION_FACTOR = weight_tared / known_weight;
    return RESULT_OK;
}

result_t read_weight(scale_t* scale, float weight) {
    if (!scale->isOn || OFFSET == 0 || CALIBRATION_FACTOR == 0) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    float weight_tared;
    read_tared(scale, weight_tared);
    weight =  weight_tared / CALIBRATION_FACTOR;
    return RESULT_OK;
}

result_t turnOff(scale_t* scale) {
    scale->isOn = false;
    return RESULT_OK;
}

static result_t read_tared(scale_t* scale, float weight_tared) {
    if (!scale->isOn) {
        return RESULT_ERR_NOT_INITIALIZED;
    }
    weight_tared = scale->raw_data - OFFSET;
    return RESULT_OK;
}

