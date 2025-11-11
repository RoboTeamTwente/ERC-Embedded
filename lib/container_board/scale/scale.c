#include "scale.h"
#include <logging.h>

static float OFFSET; //offset to tare
static float CALIBRATION_FACTOR; //reading/known weight

void init(scale_t* scale, float known_weight) {
    tare(scale);
    calibrate(scale, known_weight);
}

void tare(scale_t* scale) {
    OFFSET = scale->raw_data;
}

void calibrate(scale_t* scale, float known_weight) {
    CALIBRATION_FACTOR = read_tared(scale) / known_weight;
}

float read_weight(scale_t* scale) {
    return read_tared(scale) / CALIBRATION_FACTOR;
}

static float read_tared(scale_t* scale) {
    return scale->raw_data - OFFSET;
}

