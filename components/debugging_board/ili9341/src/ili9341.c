#include "ili9341.h"

#include "cmsis_os2.h"
#include "cubemx_main.h"
#include "ili9341_fonts.h"
#include "logging.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include "result.h"
#include "stdbool.h"

static char *TAG = "ILI9341";

#include "gpio.h"
#include "spi.h"

/* Global Variables
 * ------------------------------------------------------------------*/
volatile uint16_t LCD_HEIGHT = ILI9341_SCREEN_HEIGHT;
volatile uint16_t LCD_WIDTH = ILI9341_SCREEN_WIDTH;

/* Initialize SPI */
void ILI9341_SPI_Init(void) {
  // MX_SPI1_Init(); // SPI INIT
  // MX_GPIO_Init(); // GPIO INIT
}

/*Send data (char) to LCD*/
void ILI9341_SPI_Send(unsigned char SPI_Data) {
  HAL_SPI_Transmit(HSPI_INSTANCE, &SPI_Data, 1, 1);
}

/* Send command (char) to LCD */
void ILI9341_Write_Command(uint8_t Command) {
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
  ILI9341_SPI_Send(Command);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/* Send Data (char) to LCD */
void ILI9341_Write_Data(uint8_t Data) {
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  ILI9341_SPI_Send(Data);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/* Set Address - Location block - to draw into */
void ILI9341_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2) {
  ILI9341_Write_Command(0x2A);
  ILI9341_Write_Data(X1 >> 8);
  ILI9341_Write_Data(X1);
  ILI9341_Write_Data(X2 >> 8);
  ILI9341_Write_Data(X2);

  ILI9341_Write_Command(0x2B);
  ILI9341_Write_Data(Y1 >> 8);
  ILI9341_Write_Data(Y1);
  ILI9341_Write_Data(Y2 >> 8);
  ILI9341_Write_Data(Y2);

  ILI9341_Write_Command(0x2C);
}

/*HARDWARE RESET*/
void ILI9341_Reset(void) {
  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
  osDelay(200);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  osDelay(200);
  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}

/*Ser rotation of the screen - changes x0 and y0*/
void ILI9341_Set_Rotation(uint8_t Rotation) {

  uint8_t screen_rotation = Rotation;

  ILI9341_Write_Command(0x36);
  osDelay(1);

  switch (screen_rotation) {
  case SCREEN_VERTICAL_1:
    ILI9341_Write_Data(0x40 | 0x08);
    LCD_WIDTH = 240;
    LCD_HEIGHT = 320;
    break;
  case SCREEN_HORIZONTAL_1:
    ILI9341_Write_Data(0x20 | 0x08);
    LCD_WIDTH = 320;
    LCD_HEIGHT = 240;
    break;
  case SCREEN_VERTICAL_2:
    ILI9341_Write_Data(0x80 | 0x08);
    LCD_WIDTH = 240;
    LCD_HEIGHT = 320;
    break;
  case SCREEN_HORIZONTAL_2:
    ILI9341_Write_Data(0x40 | 0x80 | 0x20 | 0x08);
    LCD_WIDTH = 320;
    LCD_HEIGHT = 240;
    break;
  default:
    // EXIT IF SCREEN ROTATION NOT VALID!
    break;
  }
}

/*Enable LCD display*/
void ILI9341_Enable(void) {
  HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}

/*Initialize LCD display*/
void ILI9341_Init(void) {

  ILI9341_Enable();
  ILI9341_SPI_Init();
  ILI9341_Reset();

  // SOFTWARE RESET
  ILI9341_Write_Command(0x01);
  osDelay(1000);

  // POWER CONTROL A
  ILI9341_Write_Command(0xCB);
  ILI9341_Write_Data(0x39);
  ILI9341_Write_Data(0x2C);
  ILI9341_Write_Data(0x00);
  ILI9341_Write_Data(0x34);
  ILI9341_Write_Data(0x02);

  // POWER CONTROL B
  ILI9341_Write_Command(0xCF);
  ILI9341_Write_Data(0x00);
  ILI9341_Write_Data(0xC1);
  ILI9341_Write_Data(0x30);

  // DRIVER TIMING CONTROL A
  ILI9341_Write_Command(0xE8);
  ILI9341_Write_Data(0x85);
  ILI9341_Write_Data(0x00);
  ILI9341_Write_Data(0x78);

  // DRIVER TIMING CONTROL B
  ILI9341_Write_Command(0xEA);
  ILI9341_Write_Data(0x00);
  ILI9341_Write_Data(0x00);

  // POWER ON SEQUENCE CONTROL
  ILI9341_Write_Command(0xED);
  ILI9341_Write_Data(0x64);
  ILI9341_Write_Data(0x03);
  ILI9341_Write_Data(0x12);
  ILI9341_Write_Data(0x81);

  // PUMP RATIO CONTROL
  ILI9341_Write_Command(0xF7);
  ILI9341_Write_Data(0x20);

  // POWER CONTROL,VRH[5:0]
  ILI9341_Write_Command(0xC0);
  ILI9341_Write_Data(0x23);

  // POWER CONTROL,SAP[2:0];BT[3:0]
  ILI9341_Write_Command(0xC1);
  ILI9341_Write_Data(0x10);

  // VCM CONTROL
  ILI9341_Write_Command(0xC5);
  ILI9341_Write_Data(0x3E);
  ILI9341_Write_Data(0x28);

  // VCM CONTROL 2
  ILI9341_Write_Command(0xC7);
  ILI9341_Write_Data(0x86);

  // MEMORY ACCESS CONTROL
  ILI9341_Write_Command(0x36);
  ILI9341_Write_Data(0x48);

  // PIXEL FORMAT
  ILI9341_Write_Command(0x3A);
  ILI9341_Write_Data(0x55);

  // FRAME RATIO CONTROL, STANDARD RGB COLOR
  ILI9341_Write_Command(0xB1);
  ILI9341_Write_Data(0x00);
  ILI9341_Write_Data(0x18);

  // DISPLAY FUNCTION CONTROL
  ILI9341_Write_Command(0xB6);
  ILI9341_Write_Data(0x08);
  ILI9341_Write_Data(0x82);
  ILI9341_Write_Data(0x27);

  // 3GAMMA FUNCTION DISABLE
  ILI9341_Write_Command(0xF2);
  ILI9341_Write_Data(0x00);

  // GAMMA CURVE SELECTED
  ILI9341_Write_Command(0x26);
  ILI9341_Write_Data(0x01);

  // POSITIVE GAMMA CORRECTION
  ILI9341_Write_Command(0xE0);
  ILI9341_Write_Data(0x0F);
  ILI9341_Write_Data(0x31);
  ILI9341_Write_Data(0x2B);
  ILI9341_Write_Data(0x0C);
  ILI9341_Write_Data(0x0E);
  ILI9341_Write_Data(0x08);
  ILI9341_Write_Data(0x4E);
  ILI9341_Write_Data(0xF1);
  ILI9341_Write_Data(0x37);
  ILI9341_Write_Data(0x07);
  ILI9341_Write_Data(0x10);
  ILI9341_Write_Data(0x03);
  ILI9341_Write_Data(0x0E);
  ILI9341_Write_Data(0x09);
  ILI9341_Write_Data(0x00);

  // NEGATIVE GAMMA CORRECTION
  ILI9341_Write_Command(0xE1);
  ILI9341_Write_Data(0x00);
  ILI9341_Write_Data(0x0E);
  ILI9341_Write_Data(0x14);
  ILI9341_Write_Data(0x03);
  ILI9341_Write_Data(0x11);
  ILI9341_Write_Data(0x07);
  ILI9341_Write_Data(0x31);
  ILI9341_Write_Data(0xC1);
  ILI9341_Write_Data(0x48);
  ILI9341_Write_Data(0x08);
  ILI9341_Write_Data(0x0F);
  ILI9341_Write_Data(0x0C);
  ILI9341_Write_Data(0x31);
  ILI9341_Write_Data(0x36);
  ILI9341_Write_Data(0x0F);

  // EXIT SLEEP
  ILI9341_Write_Command(0x11);
  osDelay(120);

  // TURN ON DISPLAY
  ILI9341_Write_Command(0x29);

  // STARTING ROTATION
  ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
}

// INTERNAL FUNCTION OF LIBRARY, USAGE NOT RECOMENDED, USE Draw_Pixel INSTEAD
/*Sends single pixel colour information to LCD*/
void ILI9341_Draw_Colour(uint16_t Colour) {
  // SENDS COLOUR
  unsigned char TempBuffer[2] = {Colour >> 8, Colour};
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  HAL_SPI_Transmit(HSPI_INSTANCE, TempBuffer, 2, 1);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

// INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
void ILI9341_Draw_Colour_Burst(uint16_t Colour, uint32_t Size) {
  // SENDS COLOUR
  uint32_t Buffer_Size = 0;
  if ((Size * 2) < BURST_MAX_SIZE) {
    Buffer_Size = Size;
  } else {
    Buffer_Size = BURST_MAX_SIZE;
  }

  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

  unsigned char chifted = Colour >> 8;
  ;
  unsigned char burst_buffer[Buffer_Size];
  for (uint32_t j = 0; j < Buffer_Size; j += 2) {
    burst_buffer[j] = chifted;
    burst_buffer[j + 1] = Colour;
  }

  uint32_t Sending_Size = Size * 2;
  uint32_t Sending_in_Block = Sending_Size / Buffer_Size;
  uint32_t Remainder_from_block = Sending_Size % Buffer_Size;

  if (Sending_in_Block != 0) {
    for (uint32_t j = 0; j < (Sending_in_Block); j++) {
      HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char *)burst_buffer,
                       Buffer_Size, 10);
    }
  }

  // REMAINDER!
  HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char *)burst_buffer,
                   Remainder_from_block, 10);

  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/* * Sends an array of pixel data to the LCD
 * Colour: Pointer to the array of uint16_t colors
 * PixelCount: Total number of pixels to draw (NOT bytes)
 */
void ILI9341_Draw_Colour_Array(const uint16_t *Colour, uint32_t PixelCount) {
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

  const uint32_t MAX_PIXELS_PER_BURST =
      BURST_MAX_SIZE / 2; // BURST_MAX_SIZE in bytes
  static uint8_t tx_buf[BURST_MAX_SIZE];

  uint32_t pixel_index = 0;

  while (pixel_index < PixelCount) {
    uint32_t remaining_pixels = PixelCount - pixel_index;
    uint32_t pixels_to_send = (remaining_pixels > MAX_PIXELS_PER_BURST)
                                  ? MAX_PIXELS_PER_BURST
                                  : remaining_pixels;

    for (uint32_t i = 0; i < pixels_to_send; i++) {
      uint16_t c = Colour[pixel_index + i];
      tx_buf[(i * 2U) + 0U] = (uint8_t)(c >> 8);    // MSB first (big-endian)
      tx_buf[(i * 2U) + 1U] = (uint8_t)(c & 0xFFU); // LSB
    }

    HAL_SPI_Transmit(HSPI_INSTANCE, tx_buf, pixels_to_send * 2U, 100);

    pixel_index += pixels_to_send;
  }

  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void ILI9341_Draw_Bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         const uint8_t *data, uint16_t Color,
                         uint16_t bgcolor) {

  ILI9341_Set_Address(x, y, x + w - 1, y + h - 1);

  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);   // Data Mode
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); // Select Chip

  uint16_t swapped_color = (Color >> 8) | (Color << 8);
  uint16_t swapped_bgcolor = (bgcolor >> 8) | (bgcolor << 8);

  uint16_t color_buffer[BURST_MAX_SIZE]; // Buffer to hold colour data
  uint32_t buffer_index = 0;

  uint16_t byteWidth = (w + 7) / 8;

  for (uint16_t j = 0; j < h; j++) {   // Rows
    for (uint16_t i = 0; i < w; i++) { // Columns

      uint8_t byte = data[j * byteWidth + i / 8];
      uint8_t mask = 0x80 >> (i % 8);

      if (byte & mask) {
        color_buffer[buffer_index] = swapped_color;
      } else {
        color_buffer[buffer_index] = swapped_bgcolor;
      }

      buffer_index++;

      if (buffer_index >= BURST_MAX_SIZE) {
        HAL_SPI_Transmit(HSPI_INSTANCE, (uint8_t *)color_buffer,
                         buffer_index * 2, 100);

        buffer_index = 0;
      }
    }
  }

  if (buffer_index > 0) {
    HAL_SPI_Transmit(HSPI_INSTANCE, (uint8_t *)color_buffer, buffer_index * 2,
                     100);
  }
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}
// FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom
// 16bit)
/*Sets address (entire screen) and Sends Height*Width ammount of colour
 * information to LCD*/
void ILI9341_Fill_Screen(uint16_t Colour) {
  ILI9341_Set_Address(0, 0, LCD_WIDTH, LCD_HEIGHT);
  ILI9341_Draw_Colour_Burst(Colour, LCD_WIDTH * LCD_HEIGHT);
}

// DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
// Location is dependant on screen orientation. x0 and y0 locations change with
// orientations. Using pixels to draw big simple structures is not recommended
// as it is really slow Try using either rectangles or lines if possible
//
void ILI9341_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Colour) {
  if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
    return; // OUT OF BOUNDS!

  // ADDRESS
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  ILI9341_SPI_Send(0x2A);
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

  // XDATA
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  unsigned char Temp_Buffer[4] = {X >> 8, X, (X + 1) >> 8, (X + 1)};
  HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer, 4, 1);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

  // ADDRESS
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  ILI9341_SPI_Send(0x2B);
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

  // YDATA
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  unsigned char Temp_Buffer1[4] = {Y >> 8, Y, (Y + 1) >> 8, (Y + 1)};
  HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer1, 4, 1);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

  // ADDRESS
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  ILI9341_SPI_Send(0x2C);
  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

  // COLOUR
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
  unsigned char Temp_Buffer2[2] = {Colour >> 8, Colour};
  HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer2, 2, 1);
  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

// DRAW RECTANGLE OF SET SIZE AND HEIGTH AT X and Y POSITION WITH CUSTOM COLOUR
//
// Rectangle is hollow. X and Y positions mark the upper left corner of
// rectangle As with all other draw calls x0 and y0 locations dependant on
// screen orientation
void ILI9341_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width,
                            uint16_t Height, uint16_t Colour) {
  if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
    return;
  if ((X + Width - 1) >= LCD_WIDTH) {
    Width = LCD_WIDTH - X;
  }
  if ((Y + Height - 1) >= LCD_HEIGHT) {
    Height = LCD_HEIGHT - Y;
  }
  ILI9341_Set_Address(X, Y, X + Width - 1, Y + Height - 1);
  ILI9341_Draw_Colour_Burst(Colour, Height * Width);
}

// DRAW LINE FROM X,Y LOCATION to X+Width,Y LOCATION
void ILI9341_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width,
                                  uint16_t Colour) {
  if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
    return;
  if ((X + Width - 1) >= LCD_WIDTH) {
    Width = LCD_WIDTH - X;
  }
  ILI9341_Set_Address(X, Y, X + Width - 1, Y);
  ILI9341_Draw_Colour_Burst(Colour, Width);
}

// DRAW LINE FROM X,Y LOCATION to X,Y+Height LOCATION
void ILI9341_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height,
                                uint16_t Colour) {
  if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
    return;
  if ((Y + Height - 1) >= LCD_HEIGHT) {
    Height = LCD_HEIGHT - Y;
  }
  ILI9341_Set_Address(X, Y, X, Y + Height - 1);
  ILI9341_Draw_Colour_Burst(Colour, Height);
}
static void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch,
                              ILI9341_FontDef font, uint16_t color,
                              uint16_t bgcolor) {
  uint32_t i, b, j;

  ILI9341_Set_Address(x, y, x + font.width - 1, y + font.height - 1);

  for (i = 0; i < font.height; i++) {
    b = font.data[(ch - 32) * font.height + i];
    for (j = 0; j < font.width; j++) {
      if ((b << j) & 0x8000) {
        uint8_t data[] = {color >> 8, color & 0xFF};
        ILI9341_Write_Data(color >> 8);
        ILI9341_Write_Data(color & 0xFF);
      } else {
        uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
        ILI9341_Write_Data(bgcolor >> 8);
        ILI9341_Write_Data(bgcolor & 0xFF);
      }
    }
  }
}
void ILI9341_WriteString(uint16_t x, uint16_t y, const char *str,
                         ILI9341_FontDef font, uint16_t color,
                         uint16_t bgcolor) {

  while (*str) {
    if (x + font.width >= ILI9341_SCREEN_WIDTH) {
      x = 0;
      y += font.height;
      if (y + font.height >= ILI9341_SCREEN_HEIGHT) {
        break;
      }

      if (*str == ' ') {
        // skip spaces in the beginning of the new line
        str++;
        continue;
      }
    }

    ILI9341_WriteChar(x, y, *str, font, color, bgcolor);
    x += font.width;
    str++;
  }
}


static inline void print_bitmap(uint8_t **bitmap, size_t width, size_t height ) {
  uint32_t stride = (width + 7u) >> 3;
  for (uint8_t y = 0; y < height; y++) {
    for (uint8_t x = 0; x < width; x++) {
        bool set = ((*bitmap)[y * stride + (x >> 3)] & (0x80u >> (x & 7u))) != 0u;
        printf("%c", set ? '#' : '.');
    }
    printf("\n");
  }
}



static inline bool is_point_in_circle_u8(uint8_t dx, uint8_t dy, uint8_t r)
{
    uint16_t d2 = (uint16_t)dx * dx + (uint16_t)dy * dy;
    uint16_t r2 = (uint16_t)r  * r;
    return d2 <= r2;
}


static inline result_t ILI9341_Get_Rounded_Corner_Bitmap(
    uint8_t radius,
    uint8_t thickness,
    uint8_t *bitmap,
    uint32_t bitmap_bytes
){
     if (!bitmap || radius == 0u) return RESULT_ERR_INVALID_ARG;

    uint8_t rin = (thickness >= radius) ? 0u : (uint8_t)(radius - thickness);

    uint32_t stride = ((uint32_t)radius + 7u) >> 3;
    uint32_t bytes  = stride * (uint32_t)radius;

    if (bitmap_bytes < bytes) return RESULT_ERR_NO_MEM;

    memset(bitmap, 0, bytes);

    /* Pixel-center sampling, 2x fixed-point */
    int32_t cx = (int32_t)(2u * radius - 1u);
    int32_t cy = (int32_t)(2u * radius - 1u);

    uint32_t outer_r2 = (uint32_t)(2u * radius - 1u) * (uint32_t)(2u * radius - 1u);
    uint32_t inner_r2 = (rin > 0u)
        ? (uint32_t)(2u * rin - 1u) * (uint32_t)(2u * rin - 1u)
        : 0u;

    for (uint8_t y = 0; y < radius; y++) {
        for (uint8_t x = 0; x < radius; x++) {
            int32_t px = (int32_t)(2u * x + 1u);
            int32_t py = (int32_t)(2u * y + 1u);

            int32_t dx = cx - px;
            int32_t dy = cy - py;

            uint32_t d2 = (uint32_t)(dx * dx + dy * dy);

            bool in_outer = (d2 <= outer_r2);
            bool in_inner = (rin > 0u) ? (d2 < inner_r2) : false;

            if (in_outer && !in_inner) {
                bitmap[(uint32_t)y * stride + (x >> 3)] |= (uint8_t)(0x80u >> (x & 7u));
            }
        }
    }

    return RESULT_OK;
}


static inline void bitmap_rotate_90_cw_1bpp(
    const uint8_t *src, uint16_t src_w, uint16_t src_h,
    uint8_t *dst,       uint16_t dst_w, uint16_t dst_h
){
   if (!src || !dst) return;
    if ((dst_w != src_h) || (dst_h != src_w)) return;

    uint16_t src_stride = (uint16_t)((src_w + 7u) >> 3);
    uint16_t dst_stride = (uint16_t)((dst_w + 7u) >> 3);

    memset(dst, 0, (uint32_t)dst_stride * (uint32_t)dst_h);

    for (uint16_t y = 0; y < src_h; y++) {
        for (uint16_t x = 0; x < src_w; x++) {
            uint8_t src_byte = src[(uint32_t)y * src_stride + (x >> 3)];
            uint8_t src_mask = (uint8_t)(0x80u >> (x & 7u));

            if (src_byte & src_mask) {
                uint16_t dx = (uint16_t)(src_h - 1u - y);
                uint16_t dy = x;

                dst[(uint32_t)dy * dst_stride + (dx >> 3)] |= (uint8_t)(0x80u >> (dx & 7u));
            }
        }
    }}

static inline result_t ILI9341_Build_All_Rounded_Corners(
    uint8_t radius,
    uint8_t thickness,
    uint8_t *region,
    uint32_t region_bytes,
    uint8_t *out_bm[4],
    uint32_t *out_each_bytes
){
   if (!region || !out_bm || radius == 0u) return RESULT_ERR_INVALID_ARG;

    uint32_t stride = ((uint32_t)radius + 7u) >> 3;
    uint32_t each   = stride * (uint32_t)radius;
    uint32_t need   = 4u * each;

    if (region_bytes < need) return RESULT_ERR_NO_MEM;

    out_bm[0] = region + 0u * each; /* TL */
    out_bm[1] = region + 1u * each; /* TR */
    out_bm[2] = region + 2u * each; /* BR */
    out_bm[3] = region + 3u * each; /* BL */

    if (out_each_bytes) *out_each_bytes = each;

    result_t r = ILI9341_Get_Rounded_Corner_Bitmap(radius, thickness, out_bm[0], each);
    if (r != RESULT_OK) return r;

    bitmap_rotate_90_cw_1bpp(out_bm[0], radius, radius, out_bm[1], radius, radius);
    bitmap_rotate_90_cw_1bpp(out_bm[1], radius, radius, out_bm[2], radius, radius);
    bitmap_rotate_90_cw_1bpp(out_bm[2], radius, radius, out_bm[3], radius, radius);

    return RESULT_OK;
}

void ILI9341_Draw_Rectangle_Custom_Corner(uint16_t X, uint16_t Y, uint16_t Width,
                            uint16_t Height, uint8_t thickness, uint8_t corner_size, uint8_t *corners[4], uint16_t Color, uint16_t Bg_Color) {
  // draw corners
  uint32_t bitmap_size = (uint32_t) corner_size*(uint32_t) corner_size;
  uint32_t bitmap_bytes = (bitmap_size+7u)>>3;
  
  ILI9341_Draw_Bitmap(X, Y, corner_size, corner_size, corners[0], Color, Bg_Color);                                   // TL
  ILI9341_Draw_Bitmap(X + Width  - corner_size, Y, corner_size, corner_size, corners[1], Color, Bg_Color);           // TR
  ILI9341_Draw_Bitmap(X + Width  - corner_size, Y + Height - corner_size, corner_size, corner_size, corners[2], Color, Bg_Color); // BR
  ILI9341_Draw_Bitmap(X, Y + Height - corner_size, corner_size, corner_size, corners[3], Color, Bg_Color);           // BL  // Top line
  //
  //
  //
  ILI9341_Draw_Rectangle(X+corner_size, Y, Width-corner_size*2, thickness, Color);
  // Bottom Line
  ILI9341_Draw_Rectangle(X+corner_size, Y+Height-thickness, Width-corner_size*2, thickness, Color);
  // Right Line
  ILI9341_Draw_Rectangle(X, Y+corner_size, thickness, Height-corner_size*2, Color);
  // Left Line
  ILI9341_Draw_Rectangle(X+Width-thickness, Y+corner_size, thickness, Height-corner_size*2, Color);

}

result_t ILI9341_Draw_Rectangle_Rounded_Corner(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint8_t thickness, uint8_t radius, uint8_t* corner_buffer, size_t corner_buffer_size, uint16_t Colour, uint16_t Bg_Colour) {
  result_t res;
  
  uint32_t stride = ((uint32_t)radius + 7u) >> 3;
  uint32_t each   = stride * (uint32_t)radius;
  uint32_t need   = 4u * each;
  if (corner_buffer_size < need) {
    return RESULT_ERR_NO_MEM;
  }

  uint8_t *bitmaps[4] = {0};
  size_t bitmap_size_each = 0;
  TRY(ILI9341_Build_All_Rounded_Corners(radius, thickness, corner_buffer, corner_buffer_size, &bitmaps, &bitmap_size_each));
  LOGI(TAG, "Got rounded corners, each of size: %d", bitmap_size_each);

  ILI9341_Draw_Rectangle_Custom_Corner(X,Y,Width,Height,thickness, radius, bitmaps, Colour, Bg_Colour);
  return RESULT_OK;

}
