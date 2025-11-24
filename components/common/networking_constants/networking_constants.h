#ifndef NETWORKING_CONSTANTS_H
#define NETWORKING_CONSTANTS_H

// BOARD IDS

// Non-mechanical components: 0x0-0x30 (excusive)
#define MAIN_BOARD_ID 0x01
#define WIRELESS_BOARD_ID 0x02
#define SENSOR_BOARD_ID 0x03
#define PROCESSING_BOARD_ID 0x04
// Mechanical components: 0x30-0x60 (exclusive)
#define DRIVING_BOARD_ID 0x21
#define ROBOTIC_ARM_ID 0x22
#define SURFACE_SAMPLE_CONTAINER_ID 0x23

// TODO: Create a package structure for the different boards to use
#endif // !NETWORKING_CONSTANTS_H
