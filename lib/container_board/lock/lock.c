#include "lock.h"
#include <stdbool.h>
#include <logging.h>
#include <result.h>

//static vars of the system
static const uint16_t MAXANGLE = 180; //The maximum angle the servo can turn
static const uint16_t MAGNETVOLTAGE = 24; //voltage the magnet gets when actuating
static uint16_t CLOSEDANGLE; 
static uint16_t OPENANGLE; 
static uint16_t MINPW; 
static uint16_t MAXPW; 

//INIT AND UPDATE
result_t init(lock_t* lock, uint16_t closed_angle, uint16_t open_angle, uint16_t min_pulse_width, uint16_t max_pulse_width) {

    //set angles
    CLOSEDANGLE = closed_angle;
    OPENANGLE = open_angle;
    MINPW = min_pulse_width;
    MAXPW = max_pulse_width;

    //init servo
    servo_update(lock->servo, CLOSEDANGLE, calc_pulse_width(CLOSEDANGLE));

    //init magnet
    magnet_update(lock->magnet, 0);
    return RESULT_OK;
}

result_t servo_update(servo_t servo, uint16_t servo_pos, uint16_t servo_pulse_width) {
    servo.pos = servo_pos;
    servo.pulse_width = servo_pulse_width;
    return RESULT_OK;
}

result_t magnet_update(magnet_t magnet, uint16_t magnet_voltage) {
    magnet.voltage = magnet_voltage;
    return RESULT_OK;
}

result_t open(lock_t* lock) {
    lock_actuate_and_turn(lock, OPENANGLE);
    return RESULT_OK;
}

result_t close(lock_t* lock) {
    lock_actuate_and_turn(lock, CLOSEDANGLE);
    return RESULT_OK;
}

//HELPER METHODS
static result_t actuate_and_turn(lock_t* lock, uint16_t angle) {
    //set values to actuate magnet
    magnet_update(lock->magnet, MAGNETVOLTAGE);

    //set values to turn servo
    servo_update(lock->servo, angle, calc_pulse_width(angle));
    return RESULT_OK;
}

static uint16_t calc_pulse_width(uint16_t angle) {
    //Cap angle at the maximum
    if(angle>MAXANGLE) {
		angle=MAXANGLE;
	}

    // Map angle [0-MAXANGLE] to pulse [MINPULSEWIDTH-MAXPULSEWIDTH]
	return ((angle * (MAXPW - MINPW)) / MAXANGLE) + MINPW;
}