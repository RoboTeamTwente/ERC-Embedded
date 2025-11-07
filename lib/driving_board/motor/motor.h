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

/**
@brief initializes the motor by setting all fields of the motor_t struct to 0
 */
void motor_init(void);

/**
@brief for a specific motor defined by index sets all field in the motor_t struct in the motors array
@param pointer to the motors array
@param index of the motor
@
*/
void motor_update(motor_t *motors, int index,
                  float voltage,
                  float angle_of_body_frame,
                  float angular_momentum,
                  float current,
                  float rpm,
                  float direction_vector_x,
                  float direction_vector_y);
typedef struct {
    float motor_id;
    float current;
    float rpm;
    float back_emf;
} DrivingBoardMotorInformation_t;

typedef void (*MotorInfoCallback_t)(const DrivingBoardMotorInformation_t *data);//MotorInfoCallback_t is the type of any function that takes a pointer to a motor info struct and returns nothing

void Motor_RegisterCallback(MotorInfoCallback_t cb);

void MotorInfoHandler(const DrivingBoardMotorInformation_t *data);

#endif //for MOTOR_H

//TODO: write nice documentation here