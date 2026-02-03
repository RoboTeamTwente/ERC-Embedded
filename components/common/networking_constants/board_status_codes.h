#ifndef BOARD_STATUS_CODES_H
#define BOARD_STATUS_CODES_H

typedef enum {
  BOARD_STATUS_OK = 0,      // On successful state
  BOARD_STATUS_ERROR,       // General error
  BOARD_STATUS_BUSY,        // Board is busy
  BOARD_STATUS_STARTUP,     // Board is in startup phase
  BOARD_STATUS_SHUTDOWN,    // Board is shutting down
  BOARD_STATUS_MOTOR_FAIL,  // Motor failure detected
  BOARD_STATUS_SENSOR_FAIL, // Sensor failure detected
  BOARD_STATUS_COMMS_FAIL,  // Communication failure detected
} board_status_code_t;

#endif // BOARD_STATUS_CODES_H
