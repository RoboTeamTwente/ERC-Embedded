#include "logging.h"
#include <stdint.h>
#define NUM_MOTORS 10

typedef struct {
    float voltage;
    float angle_of_body_frame;
    float angular_momentum;
    float current;
    float rpm;
    float direction_vector_x;
    float direction_vector_y;
} Motor;

Motor motors[NUM_MOTORS];

void motor_init(void){//I want to put the error type from logging instead (ask Nick)

    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].voltage = 0.0f;
        motors[i].angle_of_body_frame = 0.0f;
        motors[i].angular_momentum = 0.0f;
        motors[i].current = 0.0f;
        motors[i].rpm = 0.0f;
        motors[i].direction_vector_x = 0.0f;
        motors[i].direction_vector_y = 0.0f;
    }
}

void motor_update(int index,
                  float voltage,
                  float angle_of_body_frame,
                  float angular_momentum,
                  float current,
                  float rpm,
                  float direction_vector_x,
                  float direction_vector_y) {

    if (index < 0 || index >= NUM_MOTORS) {
        LOGI("Motor", "index out of bounds");
        return; 
    }

    motors[index].voltage = voltage;
    motors[index].angle_of_body_frame = angle_of_body_frame;
    motors[index].angular_momentum = angular_momentum;
    motors[index].current = current;
    motors[index].rpm = rpm;
    motors[index].direction_vector_x = direction_vector_x;
    motors[index].direction_vector_y = direction_vector_y;
}