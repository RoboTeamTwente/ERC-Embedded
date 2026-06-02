#ifndef IP_MAC_CONSTANTS_TEST
#define IP_MAC_CONSTANTS_TEST

#define BOARD 1

#if BOARD == 1
#define TEST_SEND_IP {192, 168, 0, 111}
#define TEST_SEND_MAC {0x00, 0x80, 0xe1, 0x00, 0x00, 0x00}

#define TEST_BOARD_IP {192, 168, 0, 5}
#define TEST_BOARD_MAC {0x6C, 0x24, 0x08, 0xD2, 0xFA, 0x50}
#endif

#if BOARD == 2
#define TEST_SEND_IP {192, 168, 0, 5}
#define TEST_SEND_MAC {0x6C, 0x24, 0x08, 0xD2, 0xFA, 0x50}

#define TEST_BOARD_IP {192, 168, 0, 111}
#define TEST_BOARD_MAC {0x00, 0x80, 0xe1, 0x00, 0x00, 0x00}
#endif

#if BOARD == 3
#define TEST_SEND_IP {192, 168, 0, 5}
#define TEST_SEND_MAC {0x6C, 0x24, 0x08, 0xD2, 0xFA, 0x50}

#define TEST_BOARD_IP {192, 168, 0, 120}
#define TEST_BOARD_MAC {0x00, 0x77, 0xFE, 0xD2, 0xB9, 0xAA}
#endif

// Sending structure
// 1 -> 2
// 2 -> 1
// 3 -> 1

#endif //! IP_MAC_CONSTANTS_TEST
