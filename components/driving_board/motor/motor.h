#ifndef MOTOR_H
#define MOTOR_H

#define NUM_MOTORS 10

typedef struct {
    float motor_id;
    float actspeed;
    float control_var;//control variable for motors
} motor_speed_t;
typedef struct {
    float motor_id;
    float actangle;
    float desang;                   
    float pwnenable;                
    float pwmrev;
} motor_steering_t;
typedef struct {
    float turning_radius;
    float turning_angle;
    float distance_to_go;
    float state;
} driving_system_t;

//extern motor_t motors[NUM_MOTORS]; //number of motors will be 10

/**
@brief initializes the motor by setting all fields of the motor_t struct to 0
 */
result_t motor_init(motor_t *motors);

/**
@brief for a specific motor defined by index sets all field in the motor_t struct in the motors array
@param pointer to the motors array
@param index of the motor
*/
result_t motor_update(motor_t *motors, int index,
                  float distance_to_go,
                  float turning_radius,
                  float turning_angle);
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