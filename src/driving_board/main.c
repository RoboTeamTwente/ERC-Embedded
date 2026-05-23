#ifndef PIO_UNIT_TESTING

#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "control_drive.h"
#include "cubemx_main.h"
#include "components/common/envelope.pb.h"
//#include "diagnostics.pb.h"
//#include "motor_information.pb.h"
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/driving_board/motor_periodic_progress.pb.h"
#include "components/driving_board/motor_diagnostics.pb.h"
#include "components/common/motor.pb.h"
#include "components/driving_board/motor_msg.pb.h"
#include "ethernet.h"
#include "packet_dispatcher.h"
#include "packet_dispatcher_macros.h"
#include "ip_mac_constants.h"
#include "queue.h"
#include "netif.h"
#include "networking_constants.h"
#include "stm/ethernet_udp.h" //receive_counter
#include "gpio.h"
#include "logging.h"
#include "pb_message.h"
#include "rtwtypes.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cl3e.h"
#include <time.h>
#include <stdint.h>
#include "result.h"




#include "calculator.h"
#include <math.h>

static char *TAG = "MAIN";

extern ExtU rtU;
extern ExtY rtY;

extern void MX_FREERTOS_Init(void);

//void cubemx_main(void);
void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
void Error_Handler(void);
void MPU_Config(void);

void MX_GPIO_Init(void);
void MX_TIM1_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
extern struct netif gnetif;
extern ETH_HandleTypeDef heth;

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

//uint16_t dac_value;
void MainTask(void *argument);
void PwmTask(void *argument);
void DrivingEncoderTask(void *argument);

static float revolutions = 0;
static float radians = 0;
static float rpm = 0;

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)tskIDLE_PRIORITY,
};

const osThreadAttr_t pwmTask_attributes = {
    .name = "pwmTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)tskIDLE_PRIORITY,
};

const osThreadAttr_t drivingEncoderTask_attributes = {
    .name = "encoderTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)tskIDLE_PRIORITY,
};


void ethernet_linkstatus_callback(void *arg) {
  struct netif *netif = (struct netif *)arg;
  uint8_t ip[4] = NETWORK_IP;
  uint8_t mac[6] = SAMPEL_BOARD_MAC;
  if (netif_is_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
    ETH_add_arp(ip, mac, 5);
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

#define DISPATCHER_INPUT_QUEUE_LENGTH 8U


static int incomming_counter = 0;
static int outgoing_counter = 0;
static result_t HandleTypeMotorMsgPacket(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  DrivingBoardMotorMessage *packet = (DrivingBoardMotorMessage *)buffer;
  incomming_counter += 1;
  LOGI(TAG,"Envelope of type distance to go info has value: %f\n", packet->distance_to_go);
  LOGI(TAG, "This is packet: %d\n", incomming_counter);
  return RESULT_OK;
}



//static uint8_t packet1_buffer[DrivingBoardMotorMessage_size * 5];

/**
 * static packet_handler_config_t handler_configs[] = {
    {.handler = HandleTypeMotorMsgPacket,
     .task_name = "Motor Msg Handler",
     .packet_type = PBEnvelope_drive_motor_tag,
     .item_size = DrivingBoardMotorMessage_size,
     .task_priority = PACKET_HANDLER_PRIORITY,
     .queue_length = 5,
     .queue_buffer = packet1_buffer}};
 */


PACKET_HANDLER_CONFIG_STATIC(motor_msg_handler, PBEnvelope_drive_motor_tag, drive_motor,
                             HandleTypeMotorMsgPacket);



extern int receive_counter;


void init_board() {

  MPU_Config_wrapper();

  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  //SCB_EnableDCache();

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  osKernelInitialize();
  
  //MX_DAC1_Init();
  //HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);

  /* Initialize COM1 port */
  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  MX_USART3_Init(&huart_com, &BspCOMInit);
  LOG_init(&huart_com);
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  control_drive_initialize();

  //ethernet
  uint8_t mac[6] = NETWORK_MAC;
  uint8_t ip[4] = NETWORK_IP;
  uint8_t netmask[4] = NETMASK;
  uint8_t gateway[4] = GATEWAY;
  ETH_init(ethernet_linkstatus_callback, ip, netmask, gateway, mac);
  int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  int mac3[6] = {0x90, 0x2e, 0x16, 0xbe, 0x1b, 0x33};
  ETH_setup_MAC_address_filtering(mac1, mac2, mac3);
  

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osThreadNew(PwmTask, NULL, &pwmTask_attributes);
  osThreadNew(DrivingEncoderTask, NULL, &drivingEncoderTask_attributes);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);

  osKernelStart();


  while (1) {
  }
}




int main(void) { init_board(); }

/**
 * void FillDiagnostics(DiagnosticsData *diag)
{
    if (diag == NULL) return;

    diag->board_state = STATE_OPERATING;
    diag->motor_count = 10;

    // Front left motor
    diag->motors[0].state = STATE_OPERATING;
    diag->motors[0].motor_id = 1;
    diag->motors[0].rpm = rpm;
    diag->motors[0].voltage = 0;
    diag->motors[0].encoder_angle = radians;

    // Middle left motor
    diag->motors[1].state = STATE_OPERATING;
    diag->motors[1].motor_id = 2;
    diag->motors[1].rpm = get_motor_rpm(2);
    diag->motors[1].voltage = get_motor_voltage(2);
    diag->motors[1].encoder_angle = get_motor_angle(2);
    //add more of the motors later
}
 */


/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {//send messages calculates actual values from reallife hall sensors(encoders)

/**
 *   rtU.actspeed[0] =7.15;
  rtU.actspeed[1] =7.12;
  rtU.actspeed[2] =7.15;
  rtU.actspeed[3] =6.75;
  rtU.actspeed[4] =6.72;
  rtU.actspeed[5] =6.75;

  rtU.actang[0]=0.078;
  rtU.actang[1]=-0.078;
  rtU.actang[2]=0.109;
  rtU.actang[3]=-0.109;

  rtU.dist2goal = 2.0;  // meters
  rtU.steerang  = 1.0;
 */


  //uint8_t ip[4] = {0, 0, 0, 0};
  //uint8_t mac[6] = {255, 255, 255, 255, 255, 255};
  //ETH_udp_init();


  packet_handler_config_t handler_configs[] = {motor_msg_handler};

  int SendQueueSize = 80;
  static StaticQueue_t xStaticQueue1;
  uint8_t ucQueueStorageArea1[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t udp_receiver_queue1 =
      xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE,
                         ucQueueStorageArea1, &xStaticQueue1);

  static StaticQueue_t xStaticQueue2;
  uint8_t ucQueueStorageArea2[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t udp_receiver_queue2 =
      xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE,
                         ucQueueStorageArea2, &xStaticQueue2);
  QueueHandle_t queues[2] = {udp_receiver_queue1, udp_receiver_queue2};

  uint8_t ip[4] = SAMPLE_BOARD_IP;
  uint8_t mac[6] = SAMPEL_BOARD_MAC;

  PacketDispatcherInit(handler_configs, 1);

  ETH_udp_init(2, queues, DispatchPacket);
  ETH_add_arp(ip, mac, 5);

  /**
   * while (outgoing_counter < 1000) {
    ETH_udp_send(ip, 8, packet1_payload, 46, 1);
    osDelay(100);
    outgoing_counter += 1;
    LOGI(TAG, "%d", outgoing_counter);
  }
   */
  
 
  while (1) {
    /**
     *     ETH_udp_send(ip, 7, "udp message");
    osDelay(100);
    ETH_raw_send(mac, "ggg");
    ETH_raw_send(mac, "long ass raw message looooong looooooonger looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongest");
    osDelay(100); 
     */
    DrivingBoardDiagnostics diag =DrivingBoardDiagnostics_init_zero;

    diag.state = DrivingBoardDiagnostics_State_OPERATING;

    diag.front_left_motor.state = MotorInformation_State_OPERATING;

    diag.front_left_motor.motor_id = 1;
    diag.front_left_motor.rpm = 1200.0f;
    diag.front_left_motor.voltage = 24.0f;
    diag.front_left_motor.encoder_angle = 1.57f;

    PBEnvelope diag_envelope = PBEnvelope_init_zero;

    diag_envelope.which_payload = PBEnvelope_drive_diag_tag;

    diag_envelope.payload.drive_diag = diag;

    uint8_t *diag_encoded = NULL;
    size_t diag_size = 0;

    result_t diag_result =
        pb_message_encode(
            &diag_envelope,
            PBEnvelope_fields,
            &diag_encoded,
            &diag_size);

    if (diag_result == RESULT_OK){
        ETH_udp_send(
            ip,
            8,
            diag_encoded,
            diag_size,
            1);

        free(diag_encoded);

    LOGI(TAG,"Sent DrivingBoardDiagnostics");}
    else free(diag_encoded);  

    //send motor progress

    DrivingBoardMotorPeriodicProgress progress =
        DrivingBoardMotorPeriodicProgress_init_zero;

    progress.distance_left =
        10.0f - outgoing_counter;

    PBEnvelope progress_envelope =
        PBEnvelope_init_zero;

    progress_envelope.which_payload =
        PBEnvelope_drive_progress_tag;

    progress_envelope.payload.drive_progress =
        progress;

    uint8_t *progress_encoded = NULL;
    size_t progress_size = 0;

    result_t progress_result =
        pb_message_encode(
            &progress_envelope,
            PBEnvelope_fields,
            &progress_encoded,
            &progress_size);

    if (progress_result == RESULT_OK)
    {
        ETH_udp_send(
            ip,
            8,
            progress_encoded,
            progress_size,
            1);

        free(progress_encoded);
 
    LOGI(TAG,"Sent DrivingBoardMotorPeriodicProgress");}
    else free(progress_encoded);

    __asm__ __volatile__("nop");
    osDelay(300);
    //sending packet
    /**
     * DiagnosticsData diag;
    FillDiagnostics(&diag);
    
    uint8_t *encoded_data = NULL;
    size_t encoded_length = 0;
    result_t res = DBMDiagnosticsEncode(&diag, &encoded_data, &encoded_length);

    if (res == RESULT_OK)
    {
      ETH_udp_send(ip, 7, encoded_data);
      free(encoded_data);
    }
    else
    {
      free(encoded_data);
      LOGE(TAG, "Encoding failed for diag");
    }

    float distance_left = 10.5f;
     */

    
  }
}

void PwmTask(void *argument)
{
    uint32_t last_tick = osKernelGetTickCount();
    uint32_t wake_time = last_tick;
    const uint32_t period = 1;

    //CL3E_Test();//I ll delete it later it blocks the task bc of delays
    for(;;)
    {
      uint32_t now = osKernelGetTickCount();
      CL3E_DriveFromControl(&htim1, 1, GPIOA, 3, rtY.controlLF);
      rtU.deltaTime = (now - last_tick) * 0.001f;
      last_tick = now;
      
      control_drive_step();

      wake_time += period;// schedule next exact tick
      osDelayUntil(wake_time);
    }
}

void DrivingEncoderTask(void *argument){
  int16_t counter = 0;
  int16_t last_counter = 0;
  int16_t total_count = 0;
  const float counts_per_rev = 80.0f;//20 PPR * 4 
  const float dt = 0.1f;//100 ms
  for(;;)
  {
    counter = __HAL_TIM_GET_COUNTER(&htim4);
    int16_t delta = counter - last_counter;

    total_count += delta; 
    last_counter = counter;

    //Position
    revolutions = total_count / counts_per_rev;
    //degrees = revolutions * 360.0f;
    radians = revolutions * 2.0f * M_PI;
    // Speed
    rpm = (delta / counts_per_rev) * (60.0f / dt);

    osDelay(100); // 100 ms

    //TODO: Make this work for multiple encoders

    // For a 600 PPR encoder
    //float revolutions = count / 2400.0f;
    //float degrees = count / 2400.0f * 360.0f;
    //LOGI(TAG, "encoder revolutions: %f \n", revolutions);
    //LOGI(TAG, "encoder degrees: %f \n", degrees);
  }
}

#endif //! PIO_UNIT_TESTS
