#ifndef LOCK_LOGIC_H
#define LOCK_LOGIC_H

#include <stdint.h>

typedef struct {
    uint16_t pos;
    uint16_t pulse_width;
} servo_t;

typedef struct {
    uint16_t voltage;
} magnet_t;

typedef struct {
    servo_t servo;
    magnet_t magnet;
} lock_t;

/**
 * @brief initializes the lock
 * 
 * @param lock lock struct
 * @param closed_angle angle at which the lid is closed
 * @param open_angle angle at which the lid is open
 * @param min_pulse_width pulse width at a 1MHz clock when lid is closed
 * @param max_pulse_width pulse width at a 1MHz clock when lid is open
 */
void init(lock_t* lock, uint16_t closed_angle, uint16_t open_angle, uint16_t min_pulse_width, uint16_t max_pulse_width);

/**
 * @brief update all values in servo
 * 
 * @param servo servo struct
 * @param servo_pos angle 
 * @param servo_pulse_width pulse width
 */
void servo_update(servo_t servo, uint16_t servo_pos, uint16_t servo_pulse_width);

/**
 * @brief update all values in magnet
 * 
 * @param magnet magnet struct
 * @param magnet_voltage voltage to actuate
 */
void magnet_update(magnet_t magnet, uint16_t magnet_voltage);

/**
 * @brief opens the lock by actuating and turning to OPENANGLE
 * 
 * @param lock lock struct
 */
void open(lock_t* lock);

/**
 * @brief closes the lock by actuating and turning to CLOSEDANGLE
 * 
 * @param lock lock struct
 */
void close(lock_t* lock);

/**
 * @brief HELPER FUCTION that actuates the lock and turns the servo to a specified angle
 * 
 * @param lock lock struct
 * @param angle turn angle
 */
void actuate_and_turn(lock_t* lock, uint16_t angle);

/**
 * @brief HELPER FUNCTION that maps an angles of the servo to a pulsewidth
 * 
 * @param angle angle to turn to
 * @return uint16_t a pulse width between MINPW and MAXPW
 */
uint16_t calc_pulse_width(uint16_t angle);

#endif