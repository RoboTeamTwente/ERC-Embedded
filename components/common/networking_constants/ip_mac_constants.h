#ifndef IP_MAC_CONSTANTS
#define IP_MAC_CONSTANTS

#define SENSOR_BOARD_IP {192, 168, 0, 10}
#define SENSOR_BOARD_MAC                                                       \
  {0x00, 0x80, 0xE1, 0x00, 0x00, 0x00}

#define SENSOR_PEER_IP {192, 168, 0, 100}
#define SENSOR_PEER_MAC {0x58, 0x11, 0x22, 0x3D, 0x88, 0xFC}

#define PC_BOARD_IP {192, 168, 0, 50}
#define PC_BOARD_MAC {0x58, 0x11, 0x22, 0x3D, 0x88, 0xFC} // PC Ethernet adapter

#define SAMPLE_BOARD_IP PC_BOARD_IP
#define SAMPLE_BOARD_MAC PC_BOARD_MAC
#define SAMPEL_BOARD_MAC PC_BOARD_MAC

#define NETWORK_IP {192, 168, 0, 223}
#define NETWORK_MAC {0x00, 0x43, 0x23, 0xee, 0x21, 0x64}

#define GATEWAY {192, 168, 0, 1}
#define NETMASK {255, 255, 255, 0}
#endif //! IP_MAC_CONSTANTS
