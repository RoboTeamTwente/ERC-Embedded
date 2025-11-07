#ifndef MOTOR_H
#define MOTOR_H

#define NUM_MOTORS 10

typedef struct {
    float voltage;
    float angle_of_body_frame;
    float angular_momentum;
    float current;
    float rpm;
    float direction_vector_x;
    float direction_vector_y;
} motor_t;

extern motor_t motors[NUM_MOTORS]; //number of motors will be 10

void motor_init(void);

void motor_update(int index,
                  float voltage,
                  float angle_of_body_frame,
                  float angular_momentum,
                  float current,
                  float rpm,
                  float direction_vector_x,
                  float direction_vector_y);

#endif //for MOTOR_H