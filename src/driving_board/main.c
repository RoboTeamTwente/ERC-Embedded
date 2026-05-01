#ifndef PIO_UNIT_TESTING

#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "control.h"
#include "cubemx_main.h"
#include "components/common/envelope.pb.h"
//#include "diagnostics.pb.h"
//#include "motor_information.pb.h"
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "ethernet.h"
//#include "packet_dispatcher.h"
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
#include "bldc.h"
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
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t pwmTask_attributes = {
    .name = "pwmTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t drivingEncoderTask_attributes = {
    .name = "pwmTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};


void ethernet_linkstatus_callback(struct netif *netif) {
  if (netif_is_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

static int incomming_counter = 0;
static int outgoing_counter = 0;
static result_t HandleType1Packet(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  SensorBoardGPSInfo *packet = (SensorBoardGPSInfo *)buffer;
  incomming_counter += 1;
  printf("Envelope of type gps info has value: %f\n", packet->speed);
  printf("This is packet: %d\n", incomming_counter);
  return RESULT_OK;
}

static result_t HandleType2Packet(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardPHInfo *packet = (SensorBoardPHInfo *)buffer;
  printf("envelope of type ph info has value: %f\n", packet->ph_value);
  return RESULT_OK;
}

static uint8_t packet1_payload[] = {
    0x62, 0x2C, 0x09, 0x13, 0xF2, 0x41, 0xCF, 0x66, 0x1D, 0x4A, 0x40, 0x11,
    0x2C, 0x65, 0x19, 0xE2, 0x58, 0x97, 0x1B, 0x40, 0x1D, 0x00, 0x00, 0x0C,
    0x42, 0x2D, 0x00, 0x00, 0x87, 0x43, 0x35, 0x9A, 0x99, 0x99, 0x3F, 0x3D,
    0x66, 0x66, 0xE6, 0x3F, 0x40, 0x09, 0x48, 0x01, 0x50, 0x01};

static uint8_t packet1_buffer[SensorBoardGPSInfo_size * 5];
static uint8_t packet2_buffer[SensorBoardPHInfo_size * 5];

static packet_handler_config_t handler_configs[] = {
    {.handler = HandleType1Packet,
     .task_name = "GPS Handler",
     .packet_type = PBEnvelope_gps_info_tag,
     .item_size = SensorBoardGPSInfo_size,
     .task_priority = tskIDLE_PRIORITY + 2U,
     .queue_length = 5,
     .queue_buffer = packet1_buffer}};

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
  control_initialize();

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


/**
 * void pwm_test(){//gradually increases decreases pwm duty cycle
  
  int32_t CH1_DC = 0;

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM1_Init();
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  while (1)
  {
    while(CH1_DC < 65535)
    {
    	  TIM1->CCR1 = CH1_DC;
    	  CH1_DC += 70;
    	  HAL_Delay(1);
    }
    while(CH1_DC > 0)
    {
        TIM1->CCR1 = CH1_DC;//were writing directly to hardware register so method for updating pwm is not needed
        CH1_DC -= 70;
        HAL_Delay(1);
    }
  
    }
  }
 */



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


  rtU.actspeed[0] =7.15;
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

  //uint8_t ip[4] = {0, 0, 0, 0};
  //uint8_t mac[6] = {255, 255, 255, 255, 255, 255};
  //ETH_udp_init();

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
  while (outgoing_counter < 100) {
    ETH_udp_send(ip, 8, packet1_payload, 46, 1);
    osDelay(100);
    outgoing_counter += 1;
    LOGI(TAG, "%d", outgoing_counter);
  }
 
  while (1) {
    /**
     *     ETH_udp_send(ip, 7, "udp message");
    osDelay(100);
    ETH_raw_send(mac, "ggg");
    ETH_raw_send(mac, "long ass raw message looooong looooooonger looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongest");
    osDelay(100); 
     */


    __asm__ __volatile__("nop");
    LOGI(TAG, "Total messages send: %d", outgoing_counter);
    LOGI(TAG, "Total messages received: %d", receive_counter);
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
    
/**
 * result_t res = DBMPProgressEncode(distance_left, &encoded_data, &encoded_length);
    if (res != RESULT_OK) {
      LOGI(TAG, "Encoding failed");
    }
    else{
      LOGI(TAG, "Encoding successful");
    }
 */
    
    


/**
 *  pb_encoding_t enc;
1
    result_t res = DBMMsgEncode(10.0f, 30.0f, 2.5f, &enc);

    if (res != RESULT_OK)
    {
    //error msg
    free(enc.data);
    }
    //send with eth
    free(enc.data);
 */
/**
 * LOGI(TAG, "desspeed[0]   = %f, desspeed[1]   = %f, desspeed[2]   = %f, desspeed[3]   = %f, desspeed[4]   = %f, desspeed[5]   = %f\n", rtY.desspeed[0], rtY.desspeed[1], rtY.desspeed[2], rtY.desspeed[3], rtY.desspeed[4], rtY.desspeed[5]);
  LOGI(TAG, "controlb[0]   = %f, controlb[1]   = %f, controlb[2]   = %f, controlb[3]   = %f, controlb[4]   = %f, controlb[5]   = %f\n", rtY.controlb[0], rtY.controlb[1], rtY.controlb[2], rtY.controlb[3], rtY.controlb[4], rtY.controlb[5]);
  LOGI(TAG, "desang[0]     = %f, desang[1]     = %f, desang[2]     = %f, desang[3]     = %f\n", rtY.desang[0], rtY.desang[1], rtY.desang[2], rtY.desang[3]);
  LOGI(TAG, "pwnenable[0]  = %f, pwnenable[1]  = %f, pwnenable[2]  = %f, pwnenable[3]  = %f\n", rtY.pwnenable[0], rtY.pwnenable[1], rtY.pwnenable[2], rtY.pwnenable[3]);
  LOGI(TAG, "pwmrev[0]     = %f, pwmrev[1]     = %f, pwmrev[2]     = %f, pwmrev[3]     = %f\n", rtY.pwmrev[0], rtY.pwmrev[1], rtY.pwmrev[2], rtY.pwmrev[3]);
  
 */
  

  
  
    
     
    //BSP_LED_Toggle(LED_GREEN);
    //BSP_LED_Toggle(LED_BLUE);
    //BSP_LED_Toggle(LED_RED);
    //LOGI(TAG, "%d + %d = %d", 5, 2, add(5, 2));

   // LOGI(TAG, "This is the driving board");
    osDelay(1000);
    
  }
}

void PwmTask(void *argument){
   const uint32_t period_ms = 100;

   for(;;)
   {
       control_step();// from control.c
       //LOGI(TAG, "actang[0]     = %f, actang[1]     = %f, actang[2]     = %f, actang[3]     = %f\n", rtU.actang[0], rtU.actang[1], rtU.actang[2], rtU.actang[3]);
       //LOGI(TAG, "control step occured");
       LOGI(TAG, "pwmrev[0]   = %f, pwmrev[1]   = %f, pwmrev[2]   = %f, pwmrev[3]   = %f, pwmenable[0]   = %f, pwmenable[1]   = %f, pwmenable[2]   = %f, pwmenable[3]   = %f\n", rtY.pwmrev[0], rtY.pwmrev[1], rtY.pwmrev[2], rtY.pwmrev[3], rtY.pwnenable[0], rtY.pwnenable[1],rtY.pwnenable[2],rtY.pwnenable[3]);
       set_bldc_pwm();//this might also be done somewhere else im not sure
       //set_stepper_pwm();
       osDelay(period_ms); //fixed 1ms loop
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
