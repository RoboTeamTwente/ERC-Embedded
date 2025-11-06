#include "lock_logic.h"
#include <stdbool.h>
#include <logging.h>

//static vars of the system
static const uint16_t MAXANGLE = 180; //The maximum angle the servo can turn
static const uint16_t MAGNETVOLTAGE = 24; //voltage the magnet gets when actuating
static uint16_t CLOSEDANGLE; 
static uint16_t OPENANGLE; 
static uint16_t MINPW; 
static uint16_t MAXPW; 
//TODO: error when not initialized

//keep track of lock open or close
static bool is_open; //NOTE: i have no clue if this bool is actually useful YET

//INIT AND UPDATE
void lock_init(lock_t* lock, uint16_t closed_angle, uint16_t open_angle, uint16_t min_pulse_width, uint16_t max_pulse_width) {
    is_open = false;

    //set angles
    CLOSEDANGLE = closed_angle;
    OPENANGLE = open_angle;
    MINPW = min_pulse_width;
    MAXPW = max_pulse_width;

    //init servo
    uint16_t pos = servo_get_pos(lock->servo);
    //if not in closed postion, put it in closed position
    if (pos != CLOSEDANGLE){
        lock->servo.pulse_width = calc_pulse_width(CLOSEDANGLE);
    }
    lock->servo.pos = CLOSEDANGLE;

    //init magnet
    lock->magnet.voltage = 0;
}

// void servo_update(servo_t servo, float servo_torque, float servo_power_consumption, float servo_voltage, float servo_current, uint16_t servo_pos, uint16_t servo_pulse_width) {
//     servo.torque = servo_torque;
//     servo.power_consumption = servo_power_consumption;
//     servo.voltage = servo_voltage;
//     servo.current = servo_current;
//     servo.pos = servo_pos;
//     servo.pulse_width = servo_pulse_width;
// }

// void magnet_update(magnet_t magnet, uint16_t magnet_voltage, float magnet_current, float magnet_field_strength) {
//     magnet.voltage = magnet_voltage;
//     magnet.current = magnet_current;
//     magnet.field_strength = magnet_field_strength;
// }

void lock_open(lock_t* lock) {
    if (!is_open) {
        lock_actuate_and_turn(lock, OPENANGLE);
        is_open = true;
    }
}

void lock_close(lock_t* lock) {
    if (is_open) {
        lock_actuate_and_turn(lock, CLOSEDANGLE);
        is_open = false;
    }
}

//HELPER METHODS
void lock_actuate_and_turn(lock_t* lock, uint16_t angle) {
    //NOTE: i dont think it works like this lmao
    //set values to actuate magnet
    set_magnet_voltage(lock->magnet, MAGNETVOLTAGE); 

    //set values to turn servo
    lock->servo.pulse_width = calc_pulse_width(angle);
    lock->servo.pos = angle;

    //set values to actuate magnet
    set_magnet_voltage(lock->magnet, 0);
}

uint16_t calc_pulse_width(uint16_t angle) {
    //Cap angle at the maximum
    if(angle>MAXANGLE) {
		angle=MAXANGLE;
	}

    // Map angle [0-MAXANGLE] to pulse [MINPULSEWIDTH-MAXPULSEWIDTH]
	return ((angle * (MAXPW - MINPW)) / MAXANGLE) + MINPW;
}

//SETTERS N GETTERS
uint16_t get_servo_pos(servo_t servo) {
    return servo.pos;
}

void set_magnet_voltage(magnet_t magnet, uint16_t volt) {
    magnet.voltage = volt;
}

uint16_t get_magnet_voltage(magnet_t magnet) {
    return magnet.voltage;
}