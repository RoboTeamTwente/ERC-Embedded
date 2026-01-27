#ifndef UNIT_TEST
#include "button_matrix_driver.h"
#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "ili9341.h"
#include "ili9341_fonts.h"
#include "logging.h"
#include "menu_driver.h"
#include "menu_driver_icons.h"
#include "menu_driver_imgs.h"
#include "menu_driver_list.h"
#include "pb_message.h"
#include "result.h"
#include "string.h"

#include "FreeRTOS.h"
#include "bucketed_pqueue.h"
#include "task.h"

static char *TAG = "MAIN";

void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
void MainTask(void *argument);

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityNormal,
};
typedef struct {
  uint32_t seq;
  uint32_t tick;
  uint32_t payload;
} test_item_t;

#define BQ_NUM_BUCKETS (4U)
#define BQ_BUCKET_LEN (8U) /* items per bucket */
#define PRODUCER_PERIOD_MS (50U)
#define CONSUMER_PERIOD_MS (10U) /* Static backing buffer for queue storage */

typedef struct {
  uint32_t seq;
  uint8_t prio;
  uint32_t tick;
} bq_item_t;

/* ---------- Globals ---------- */

static QueueHandle_t g_buckets[BQ_NUM_BUCKETS];
static bucketed_pqueue_t g_bq;

static void vBQProducerTask(void *arg) {
  (void)arg;

  uint32_t seq = 0U;

  for (;;) {
    bq_item_t item;
    item.seq = seq++;
    item.tick = (uint32_t)xTaskGetTickCount();

    /* Simple pattern: cycle priorities 0..(N-1) */
    item.prio = (uint8_t)(item.seq % BQ_NUM_BUCKETS);

    /* Try enqueue; keep it non-blocking for interactive testing */
    result_t r = bucketed_pqueue_push(&g_bq, item.prio, &item, 0U);

    if (r == RESULT_OK) {
      LOGI("push prio=%u seq=%lu tick=%lu", (unsigned)item.prio,
           (unsigned long)item.seq, (unsigned long)item.tick);
    } else if (r == RESULT_ERR_OVERFLOW) {
      LOGE("push overflow prio=%u", (unsigned)item.prio);
    } else {
      LOGE("push err=%d", (int)r);
    }

    vTaskDelay(pdMS_TO_TICKS(PRODUCER_PERIOD_MS));
  }
}

static void vBQConsumerTask(void *arg) {
  (void)arg;

  uint32_t last_seq[BQ_NUM_BUCKETS] = {0U};
  uint8_t seen[BQ_NUM_BUCKETS] = {0U};

  for (;;) {
    bq_item_t item;

    result_t r = bucketed_pqueue_pop(&g_bq, &item);
    if (r == RESULT_OK) {
      /* Per-priority monotonic check (basic sanity) */
      if (seen[item.prio] != 0U && item.seq <= last_seq[item.prio]) {
        LOGE("order violation prio=%u seq=%lu last=%lu", (unsigned)item.prio,
             (unsigned long)item.seq, (unsigned long)last_seq[item.prio]);
      }
      last_seq[item.prio] = item.seq;
      seen[item.prio] = 1U;

      LOGI("pop  prio=%u seq=%lu tick=%lu now=%lu", (unsigned)item.prio,
           (unsigned long)item.seq, (unsigned long)item.tick,
           (unsigned long)xTaskGetTickCount());
    } else if (r != RESULT_ERR_NOT_FOUND) {
      LOGE("pop err=%d", (int)r);
    }

    vTaskDelay(pdMS_TO_TICKS(CONSUMER_PERIOD_MS));
  }
}

static void vBQNotifyConsumerTask(void *arg) {
  (void)arg;

  for (;;) {
    /* Wait until at least one push notifies us */
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (;;) {
      bq_item_t item;
      result_t r = bucketed_pqueue_pop(&g_bq, &item);
      if (r != RESULT_OK) {
        break;
      }
      LOGI("pop(notify) prio=%u seq=%lu", (unsigned)item.prio,
           (unsigned long)item.seq);
    }
  }
}

SPI_HandleTypeDef hspi1;
static const uint8_t main_menu_icons[3][MENU_DRIVER_ICON_BYTE_SIZE] = {
    {0xff, 0xe3, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xff,
     0xff, 0x81, 0xff, 0xff, 0xff, 0x19, 0xff, 0xff, 0xff, 0x39, 0xff, 0xff,
     0xfe, 0x39, 0xff, 0xff, 0xfc, 0x79, 0xff, 0xff, 0xfc, 0xf9, 0xff, 0xff,
     0xf8, 0xf8, 0xff, 0xff, 0xf9, 0xfc, 0xff, 0xff, 0xf1, 0xfc, 0xff, 0xff,
     0xe3, 0xfc, 0xff, 0xff, 0xe7, 0xfc, 0xff, 0xff, 0xc7, 0xfc, 0xff, 0xff,
     0x0f, 0xfc, 0x7f, 0xf8, 0x1f, 0xfe, 0x7f, 0xf0, 0xff, 0xfe, 0x7f, 0xe3,
     0xff, 0xfe, 0x7f, 0xe7, 0xff, 0xfe, 0x7f, 0xc7, 0xff, 0xfe, 0x7f, 0x8f,
     0xff, 0xfe, 0x3f, 0x9f, 0xff, 0xff, 0x3f, 0x1f, 0xff, 0xff, 0x3e, 0x3f,
     0xff, 0xff, 0x3e, 0x7f, 0xff, 0xff, 0x3c, 0x7f, 0xff, 0xff, 0x3c, 0xff,
     0xff, 0xff, 0x18, 0xff, 0xff, 0xff, 0x91, 0xff, 0xff, 0xff, 0x93, 0xff,
     0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0x87, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xef, 0xff, 0xff, 0xff, 0xbf, 0xff,
     0xff, 0xff, 0xdf, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff,
     0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
     0xff, 0xf8, 0x1f, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xfc, 0x3f, 0xff,
     0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff,
     0xff, 0xfc, 0x3f, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xf8, 0x1f, 0xff,
     0xff, 0xf0, 0x0f, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xc0, 0x03, 0xff,
     0xff, 0xc0, 0x03, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0x01, 0xff,
     0xff, 0x80, 0x01, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff,
     0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x80, 0x01, 0xff,
     0xff, 0xc0, 0x03, 0xff, 0xff, 0xe0, 0x07, 0xff},
    {0x80, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f,
     0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x3f,
     0x80, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x3f,
     0xc0, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x1f,
     0xc0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x0f,
     0xe0, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x07,
     0xf0, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x07,
     0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x03,
     0xf8, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x01,
     0xfc, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00,
     0xfe, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00}

};
static menu_page_state main_menu_state = {.list = {
                                              .num_entries = 3,
                                              .selected_index = 0,
                                              .entry_ids = {1, 2, 3},
                                              .entry_icons = main_menu_icons,
                                          }};

static menu_page_t pages[10] = {[0] =
                                    {
                                        .name = "Main Menu",
                                        .state = &main_menu_state,
                                        .id = 0,
                                        .needs_render = true,
                                        .render = menu_page_render_list,

                                    },
                                [1] = {.name = "Diagnostics"},
                                [2] = {.name = "Testing"},
                                [3] = {.name = "Logs"}};

static menu_manager_t manager = {
    .active_page_id = 0,
    .pages = &pages,
    .get_input = 0,
};

void MainTask(void *argument) {
  LOGI(TAG, "Starting Main Task\n");
  LOGI(TAG, "Initializing ILI9341\n");

  HAL_GPIO_WritePin(MATRIX_COL_A_GPIO_Port, MATRIX_COL_A_Pin, GPIO_PIN_RESET);
  while (1) {
    osDelay(1000);
  }
  ILI9341_Init();
  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
  // ILI9341_Set_Address(40, 0, ILI9341_SCREEN_WIDTH - 1,
  //                     ILI9341_SCREEN_HEIGHT - 1);
  // ILI9341_Draw_Colour_Array(menu_driver_img_ludwig_1, 280 * 240);

  // // menu_page_t active_page = manager.pages[manager.active_page_id];
  // // active_page.init(active_page.state);
  // // LOGI(TAG, "Rendering Active Page\n");
  // // active_page.render(&manager);
  // // LOGI(TAG, "Active Page Rendered\n");
  //
  // // ILI9341_Fill_Screen(BLACK);
  menu_driver_draw_ribbon();
  // ILI9341_WriteString(65, 200, "Oopsie, rover dead :D", ILI9341_Font_11x18,
  //                     MENU_DRIVER_FOREGROUND_COLOR,
  //                     MENU_DRIVER_BACKGROUND_COLOR);
  menu_page_render_list(&manager);
  while (1) {
    if (pages[0].needs_render) {
      menu_page_render_list(&manager);
      pages[0].needs_render = false;
    }

    button_matrix_input_t input = ButtonMatrixDriver_Scan();
    if (input.row != 0 && input.col != 0) {
      LOGI(TAG, "Button Pressed at Row: %d, Col: %d\n", input.row, input.col);
      main_menu_state.list.selected_index =
          (main_menu_state.list.selected_index + 1) %
          main_menu_state.list.num_entries;
      pages[0].needs_render = true;
    }
    // LOGI(TAG, "ILI9341 Initialized and Running\n");
    osDelay(80);
  }
}

void main() {
  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();

  MX_GPIO_Init();
  SystemClock_Config();
  MX_SPI1_Init();

  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); // CS OFF

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

  osKernelInitialize();
  LOGI(TAG, "Kernel Initialized");
  while(1) {
    osDelay(100);
  }
  for (uint8_t i = 0U; i < BQ_NUM_BUCKETS; i++) {
    g_buckets[i] = xQueueCreate(BQ_BUCKET_LEN, sizeof(bq_item_t));
    if (g_buckets[i] == NULL) {
      LOGE("xQueueCreate failed for bucket %u", (unsigned)i);
      for (;;) {
      }
    }
  }

  /* Option A: polling consumer */
  {
    TaskHandle_t cons_handle = NULL;
    (void)xTaskCreate(vBQNotifyConsumerTask, "bq_ncons", 512U, NULL, 2U,
                      &cons_handle);

    result_t r =
        bucketed_pqueue_init(&g_bq, g_buckets, BQ_NUM_BUCKETS, cons_handle);
    if (r != RESULT_OK) {
      for (;;) {
      }
    }

    (void)xTaskCreate(vBQProducerTask, "bq_prod", 512U, NULL, 1U, NULL);
  }
  osKernelStart();

  while (1) {
  }
}

#endif //! UNIT_TEST
//
