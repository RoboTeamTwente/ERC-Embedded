#ifndef PIO_UNIT_TESTING

#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "control_drive_manual.h"
#include "cubemx_main.h"
#include "components/common/envelope.pb.h"
//#include "diagnostics.pb.h"
//#include "motor_information.pb.h"
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/driving_board/motor_periodic_progress.pb.h"
#include "components/driving_board/motor_diagnostics.pb.h"
#include "components/basestation/manual_brake.pb.h"
#include "components/basestation/manual_drive.pb.h"
#include "components/common/motor_driver/cubemars_ak/cubemars_ak.h"
#include "fdcan.h"
#include "usart.h"
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
#include "stepper.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cl3e.h"
#include <time.h>
#include <stdint.h>
#include "result.h"





#include "calculator.h"
#include <math.h>

#define LF_ID 93//placeholder should definitely change
#define LM_ID 2
#define LB_ID 3

#define RF_ID 4
#define RM_ID 5
#define RB_ID 6

stepper_t stepperLF;
stepper_t stepperLB;
stepper_t stepperRF;
stepper_t stepperRB;

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
void MX_TIM5_Init(void);
extern struct netif gnetif;
extern ETH_HandleTypeDef heth;

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

//uint16_t dac_value;

void MainTask(void *argument);
void PwmTask(void *argument);
void DrivingEncoderTask(void *argument);
void DriveTask(void *argument);
void MainTaskListener(void *argument);


// Task attributes for CMSIS-RTOS v2

const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};

const osThreadAttr_t pwmTask_attributes = {
    .name = "pwmTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};

const osThreadAttr_t drivingEncoderTask_attributes = {
    .name = "encoderTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};


const osThreadAttr_t driveTask_attributes = {
    .name = "driveTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};

const osThreadAttr_t mainTaskListener_attributes = {
    .name = "mainTaskListener",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
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
  LOGI(TAG, "MotorMsg Packet This is packet: %d\n", incomming_counter);
  return RESULT_OK;
}

static result_t HandleTypeManualDrivePacket(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  BasestationManualDrive *packet = (BasestationManualDrive *)buffer;
  incomming_counter += 1;
  
  rtU.controllerSpeed = packet->forward_backward;
  rtU.controllerSteering = packet->turn;

  LOGI(TAG,"Envelope of type forward_backward to go info has value: %f\n", packet->forward_backward);
  LOGI(TAG,"Envelope of type turn to go info has value: %f\n", packet->turn);
  LOGI(TAG, "BasestationManualDrive This is packet: %d\n", incomming_counter);
  return RESULT_OK;
}

static result_t HandleTypeManualBrakePacket(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  BasestationManualBrake *packet = (BasestationManualBrake *)buffer;
  incomming_counter += 1;
  
  rtU.break_d = packet->brake;
  LOGI(TAG,"Envelope of type brake info has value: %f\n", packet->brake);
  LOGI(TAG, "ManualBrakePacket This is packet: %d\n", incomming_counter);
  return RESULT_OK;
}


PACKET_HANDLER_CONFIG_STATIC(motor_msg_handler, PBEnvelope_drive_motor_tag, drive_motor,
                             HandleTypeMotorMsgPacket);

PACKET_HANDLER_CONFIG_STATIC(manual_drive_handler, PBEnvelope_manual_drive_tag, manual_drive,
                             HandleTypeManualDrivePacket);

PACKET_HANDLER_CONFIG_STATIC(manual_brake_handler, PBEnvelope_manual_brake_tag, manual_brake,
                             HandleTypeManualBrakePacket);

extern int receive_counter;

static void CAN_ConfigRx_AllStandard(void) {
  FDCAN_FilterTypeDef filter = {0};

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

  // Accept everything: (ID & 0x000) == (0x000 & 0x000)
  filter.FilterID1 = 0x000;
  filter.FilterID2 = 0x000;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK) {
    LOGE("CAN", "Filter config failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(
          &hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
          FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
    LOGE("CAN", "Global filter failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
}

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan,
                                        uint32_t BufferIndexes) {
  LOGI("CAN", "TX complete buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan,
                                     uint32_t BufferIndexes) {
  LOGI("CAN", "TX abort buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
  LOGE("CAN", "Error callback HALerr=0x%08lx\n", HAL_FDCAN_GetError(hfdcan));
}

static cubemars_ak_information motor_info = {0};
static volatile cubemars_ak_information motors[256] = {0};//TODO:NOT BEING UPDATED RN CHANGE LATER

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0) {
    return;
  }

  FDCAN_RxHeaderTypeDef rx_header = {0};
  uint8_t rx_data[8] = {0};

  if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) !=
      HAL_OK) {
    LOGE("CAN", "RX read failed, err=0x%08lx", HAL_FDCAN_GetError(hfdcan));
    return;
  }
  //cubemars_ak_parse_can_feedback(&rx_header, rx_data, &motor_info);

  //cl3e_parse_can_message(&rx_header, rx_data);
}



static void fill_motor(MotorInformation *m, int id)
{
    m->state = MotorInformation_State_OPERATING;
    m->motor_id = id;

    m->rpm = motors[id].motor_speed;
    m->voltage = 24.0f;
    m->encoder_angle = motors[id].motor_position;
}




void init_board() {

  MPU_Config_wrapper();

  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_TIM5_Init();
  MX_FDCAN1_Init();
  MX_USART2_UART_Init();
  control_drive_manual_initialize();

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
  

  CAN_ConfigRx_AllStandard();
  if (HAL_FDCAN_ConfigInterruptLines(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     FDCAN_INTERRUPT_LINE0) != HAL_OK) {
    LOGE("CAN", "Interrupt line config failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     0) != HAL_OK) {
    LOGE("CAN", "Activate RX notification failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    LOGE(TAG, "FDCAN start failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
    for (;;)
      ;
  }
  LOGI("CAN", "Mode=%lu Presc=%lu TS1=%lu TS2=%lu SJW=%lu", hfdcan1.Init.Mode,
       hfdcan1.Init.NominalPrescaler, hfdcan1.Init.NominalTimeSeg1,
       hfdcan1.Init.NominalTimeSeg2, hfdcan1.Init.NominalSyncJumpWidth);

       
  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osThreadNew(PwmTask, NULL, &pwmTask_attributes);
  osThreadNew(DrivingEncoderTask, NULL, &drivingEncoderTask_attributes);
  osThreadNew(DriveTask, NULL, &driveTask_attributes);
  osThreadNew(MainTaskListener, NULL, &mainTaskListener_attributes);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);

  //HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);

  osKernelStart();


  while (1) {
  }
}


int main(void) { init_board(); }


void MainTaskListener(void *argument) {
  LOGI(TAG, "Listener Task");
  for (;;) {
    LOGI(TAG, "Listening...");
    LOGI("CAN", "RX FIFO0 fill=%lu",
         HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0));
    osDelay(5000);
  }
}


/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {//send messages calculates actual values from reallife hall sensors(encoders)

  packet_handler_config_t handler_configs[] = {motor_msg_handler, manual_drive_handler, manual_brake_handler};

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

  PacketDispatcherInit(handler_configs, 3);

  ETH_udp_init(2, queues, DispatchPacket);
  ETH_add_arp(ip, mac, 5);
  
 
  while (1) {

    DrivingBoardDiagnostics diag =DrivingBoardDiagnostics_init_zero;

    diag.state = DrivingBoardDiagnostics_State_OPERATING;

    fill_motor(&diag.front_left_motor, LF_ID);
    fill_motor(&diag.middle_left_motor, LM_ID);
    fill_motor(&diag.back_left_motor, LB_ID);
    fill_motor(&diag.front_right_motor, RF_ID);
    fill_motor(&diag.middle_right_motor, RM_ID);
    fill_motor(&diag.back_right_motor, RB_ID);

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
      
      //control_drive_manual_step();

      wake_time += period;// schedule next exact tick
      osDelayUntil(wake_time);
    }
}

void DrivingEncoderTask(void *argument){

  for(;;)
  {
    cl3e_request_position(&hfdcan1, 1);
    cl3e_request_position(&hfdcan1, 2);
    osDelay(1000);

    LOGI("CL3E: motor 0", "pos=%ld", g_cl3e_info[0].actual_position);
  }
}
void DriveTask(void *argument)
{
    osDelay(3000);
    init_stepper(&stepperLF, 50, &htim1);
    init_stepper(&stepperLB, 50, &htim3);
    init_stepper(&stepperRF, 50, &htim4);
    init_stepper(&stepperRB, 50, &htim5);

    for (;;)
    {
        LOGI("TEST", "sending");

        cubemars_ak_set_speed(&hfdcan1, 93, 10000);
        osDelay(1000);

        cubemars_ak_set_speed(&hfdcan1, 93, 0);
        osDelay(1000);
        //uint32_t now = osKernelGetTickCount();


       //control_drive_manual_step();
       /*
        * LEFT FRONT
        */
       rtU.LFActualPos =rtU.RFActualPos;

          



       /*
        * RUN CONTROLLER
        */
      


       /*
        * SEND OUTPUTS TO MOTORS
        */




           /**
            *
       cubemars_ak_set_speed(
           &hfdcan1,
           LF_ID,
           (int32_t)rtY.controlLF);


       cubemars_ak_set_speed(
           &hfdcan1,
           LM_ID,
           (int32_t)rtY.controlLM);


       cubemars_ak_set_speed(
           &hfdcan1,
           LB_ID,
           (int32_t)rtY.controlLB);


       cubemars_ak_set_speed(
           &hfdcan1,
           RF_ID,
           (int32_t)rtY.controlRF);


       cubemars_ak_set_speed(
           &hfdcan1,
           RM_ID,
           (int32_t)rtY.controlRM);


       cubemars_ak_set_speed(
           &hfdcan1,
           RB_ID,
           (int32_t)rtY.controlRB);
            */
      
         LOGI("AK1",
       "pos=%.1f deg speed=%d",
       motors[1].motor_position / 10.0f,
       motors[1].motor_speed);


               /*
       * STEPPER CONTROL
       */

/**
 * // LEFT FRONT
       rotate_stepper(
           &stepperLF,
           (uint8_t)fabs(rtY.stepperLFSteps),
           (uint32_t)fabs(rtY.stepperLFFrequency)
       );


       // LEFT BACK
       rotate_stepper(
           &stepperLB,
           (uint8_t)fabs(rtY.stepperLBSteps),
           (uint32_t)fabs(rtY.stepperLBFrequency)
       );


       // RIGHT FRONT
       rotate_stepper(
           &stepperRF,
           (uint8_t)fabs(rtY.stepperRFSteps),
           (uint32_t)fabs(rtY.stepperRFFrequency)
       );


       // RIGHT BACK
       rotate_stepper(
           &stepperRB,
           (uint8_t)fabs(rtY.stepperRBSteps),
           (uint32_t)fabs(rtY.stepperRBFrequency)
       );
 */
       


        osDelay(10);
    }
}

#endif //! PIO_UNIT_TESTS
