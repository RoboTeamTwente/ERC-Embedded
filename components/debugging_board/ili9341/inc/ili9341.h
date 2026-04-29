#ifndef ILI9341_H
#define ILI9341_H

#include "ili9341_fonts.h"
#include "result.h"
#include "stm32h7xx_hal.h"

#define ILI9341_SCREEN_HEIGHT 240
#define ILI9341_SCREEN_WIDTH 320

// SPI INSTANCE
#define HSPI_INSTANCE &hspi1

#define TFT_CS_GPIO_Port NULL
#define TFT_CS_Pin NULL
#define TFT_DC_GPIO_Port NULL
#define TFT_DC_Pin NULL
#define TFT_RESET_GPIO_Port NULL
#define TFT_RESET_Pin NULL

// CHIP SELECT PIN AND PORT, STANDARD GPIO
#define LCD_CS_PORT TFT_CS_GPIO_Port
#define LCD_CS_PIN TFT_CS_Pin

// DATA COMMAND PIN AND PORT, STANDARD GPIO
#define LCD_DC_PORT TFT_DC_GPIO_Port
#define LCD_DC_PIN TFT_DC_Pin

// RESET PIN AND PORT, STANDARD GPIO
#define LCD_RST_PORT TFT_RESET_GPIO_Port
#define LCD_RST_PIN TFT_RESET_Pin

#define BURST_MAX_SIZE 500

#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F

#define SCREEN_VERTICAL_1 0
#define SCREEN_HORIZONTAL_1 1
#define SCREEN_VERTICAL_2 2
#define SCREEN_HORIZONTAL_2 3

void ILI9341_SPI_Init(void);
void ILI9341_SPI_Send(unsigned char SPI_Data);
void ILI9341_Write_Command(uint8_t Command);
void ILI9341_Write_Data(uint8_t Data);
void ILI9341_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
void ILI9341_Reset(void);
void ILI9341_Set_Rotation(uint8_t Rotation);
void ILI9341_Enable(void);
void ILI9341_Init(void);
void ILI9341_Fill_Screen(uint16_t Colour);
void ILI9341_Draw_Colour(uint16_t Colour);
void ILI9341_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Colour);
void ILI9341_Draw_Colour_Burst(uint16_t Colour, uint32_t Size);

void ILI9341_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width,
                            uint16_t Height, uint16_t Colour);
void ILI9341_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width,
                                  uint16_t Colour);
void ILI9341_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height,
                                uint16_t Colour);

void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str,
                         ILI9341_FontDef font, uint16_t color,
                         uint16_t bgcolor);

void ILI9341_Draw_Colour_Array(const uint16_t* Colour, uint32_t PixelCount);
void ILI9341_Draw_Bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         const uint8_t* bitmap, uint16_t Color,
                         uint16_t BgColor);

void ILI9341_Draw_Rounded_Rectangle(uint16_t X, uint16_t Y, uint16_t Width,
                                    uint16_t Height, uint8_t thickness,
                                    uint8_t radius, uint16_t Colour);

result_t ILI9341_Draw_Rectangle_Rounded_Corner(
    uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint8_t thickness,
    uint8_t radius, uint8_t* corner_buffer, size_t corner_buffer_size,
    uint16_t Colour, uint16_t Bg_Colour);
#endif
