
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp8266/gpio_struct.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_libc.h"

#include "driver/gpio.h"
#include "driver/spi.h"
#include "esp8266/spi_struct.h"

static const char *TAG = "spi_lcd";
#define LCD_SCL_GPIO    14
#define LCD_SDA_GPIO    13
#define LCD_DC_GPIO     12
#define LCD_RST_GPIO    15
#define LCD_PIN_SEL  (1ULL<<LCD_DC_GPIO) | (1ULL<<LCD_RST_GPIO)

#define BLACK   0x0000    // 黑色     0,     0,     0
#define NAVY    0x000F    // 深蓝色   0,     0, 127
#define DGREEN  0x03E0    // 深绿色   0, 127, 0
#define DCYAN   0x03EF    // 深青色   0, 127, 127           
#define MAROON  0x7800    // 深红色   127,     0,     0            
#define PURPLE  0x780F    // 紫色     127,     0, 127            
#define OLIVE   0x7BE0    // 橄榄绿   127, 127,     0            
#define LGRAY   0xC618    // 灰白色   192, 192, 192            
#define DGRAY   0x7BEF    // 深灰色   127, 127, 127            
#define BLUE    0x001F    // 蓝色     0,     0, 255                
#define GREEN   0x07E0    // 绿色     0, 255,     0                
#define CYAN    0x07FF    // 青色     0, 255, 255                
#define RED     0xF800    // 红色     255,     0,     0            
#define MAGENTA 0xF81F    // 品红     255,     0, 255            
#define YELLOW  0xFFE0    // 黄色     255, 255, 0                
#define WHITE   0xFFFF    // 白色     255, 255, 255        

const uint8_t gImage_qq_logo[3200] = { /* 0X00,0X10,0X28,0X00,0X28,0X00,0X01,0X1B, */
    0XDB, 0XDE, 0X5D, 0XEF, 0X5D, 0XEF, 0X7D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF,
    0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0XBE, 0XF7,
    0XFF, 0XFF, 0XDE, 0XFF, 0X38, 0XC6, 0X92, 0X8C, 0X8E, 0X6B, 0X6E, 0X6B, 0X10, 0X7C, 0X96, 0XAD,
    0X3C, 0XE7, 0XFF, 0XFF, 0XFF, 0XFF, 0X7D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF,
    0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF, 0X5D, 0XEF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X5D, 0XEF,
    0X15, 0X9D, 0X4F, 0X63, 0X6C, 0X42, 0X0A, 0X32, 0X88, 0X29, 0X46, 0X19, 0X25, 0X19, 0X45, 0X21,
    0XE8, 0X31, 0X8E, 0X6B, 0X18, 0XBE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0XFF, 0XF5, 0X9C, 0X0F, 0X53,
    0X10, 0X4B, 0X51, 0X53, 0X0F, 0X4B, 0X6C, 0X3A, 0XE9, 0X31, 0X67, 0X21, 0X25, 0X19, 0XE4, 0X10,
    0XA3, 0X08, 0X62, 0X00, 0X83, 0X08, 0X8A, 0X4A, 0X59, 0XC6, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X1C, 0XE7, 0X4F, 0X5B, 0XB3, 0X63, 0XB8, 0X7C,
    0XF5, 0X63, 0X11, 0X43, 0X4D, 0X32, 0XEA, 0X29, 0X88, 0X21, 0X26, 0X19, 0X05, 0X19, 0X05, 0X19,
    0X04, 0X11, 0X04, 0X11, 0XE4, 0X10, 0X83, 0X00, 0XA3, 0X08, 0X72, 0X8C, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDB, 0XDE, 0X4B, 0X3A, 0XF0, 0X42, 0X35, 0X6C, 0X54, 0X4B,
    0XB1, 0X32, 0X2E, 0X2A, 0XEB, 0X21, 0XA9, 0X21, 0X67, 0X19, 0X05, 0X19, 0X04, 0X11, 0X04, 0X11,
    0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X05, 0X19, 0XE4, 0X10, 0X42, 0X00, 0XAF, 0X73, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0X5D, 0XEF, 0X09, 0X32, 0X4C, 0X32, 0X10, 0X4B, 0X8F, 0X32, 0X4F, 0X2A,
    0X2E, 0X2A, 0XCC, 0X19, 0X89, 0X19, 0X89, 0X21, 0X47, 0X19, 0X05, 0X19, 0X04, 0X11, 0X04, 0X11,
    0XC4, 0X10, 0XC4, 0X10, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0XE4, 0X10, 0X42, 0X00, 0X31, 0X84,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XEC, 0X52, 0X47, 0X19, 0X4C, 0X32, 0X0B, 0X2A, 0XEC, 0X21, 0XEC, 0X21,
    0X0C, 0X22, 0X91, 0X5B, 0XEE, 0X4A, 0X06, 0X11, 0X26, 0X19, 0X04, 0X19, 0XE4, 0X10, 0XE4, 0X10,
    0XA7, 0X29, 0X66, 0X21, 0XA3, 0X08, 0X05, 0X19, 0X04, 0X11, 0X04, 0X11, 0XE4, 0X10, 0X82, 0X00,
    0XF7, 0XBD, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0X35, 0XA5, 0X83, 0X08, 0X88, 0X21, 0X88, 0X21, 0X89, 0X21, 0XAA, 0X21, 0X8A, 0X21,
    0X6B, 0X42, 0X71, 0X8C, 0XFF, 0XFF, 0X72, 0X8C, 0X83, 0X08, 0X04, 0X11, 0XC4, 0X08, 0X29, 0X42,
    0XFB, 0XDE, 0X5D, 0XEF, 0XEC, 0X5A, 0X83, 0X08, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X83, 0X08,
    0XE8, 0X31, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XBE, 0XF7, 0XC7, 0X31, 0XC4, 0X10, 0X25, 0X19, 0X26, 0X19, 0X47, 0X19, 0X47, 0X19, 0XA8, 0X29,
    0X8A, 0X52, 0X28, 0X4A, 0X55, 0XAD, 0XFF, 0XFF, 0XE8, 0X31, 0XA3, 0X08, 0X05, 0X19, 0X4D, 0X6B,
    0X4D, 0X6B, 0XFF, 0XFF, 0X7D, 0XEF, 0X45, 0X21, 0XC4, 0X10, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11,
    0X62, 0X00, 0X76, 0XAD, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X96, 0XB5, 0X62, 0X00, 0X04, 0X11, 0X04, 0X19, 0X05, 0X11, 0X05, 0X19, 0XC4, 0X08, 0X8B, 0X4A,
    0XB6, 0XB5, 0X5D, 0XEF, 0XF7, 0XBD, 0XFF, 0XFF, 0X8E, 0X6B, 0X62, 0X00, 0X29, 0X42, 0XAA, 0X5A,
    0X08, 0X42, 0XFF, 0XFF, 0XFF, 0XFF, 0XCC, 0X52, 0X83, 0X08, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11,
    0XA3, 0X08, 0XAD, 0X52, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XBE, 0XF7, 0XFF, 0XFF, 0XDF, 0XFF, 0XDB, 0XDE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X4E, 0X63, 0X62, 0X00, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0XE4, 0X10, 0X62, 0X00, 0X8E, 0X63,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XCF, 0X73, 0X01, 0X00, 0XF3, 0X9C, 0X2C, 0X63,
    0X96, 0XB5, 0XFF, 0XFF, 0XFF, 0XFF, 0X2D, 0X5B, 0X83, 0X00, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11,
    0XE4, 0X10, 0X67, 0X21, 0X3D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XB7, 0XB5, 0X76, 0XAD, 0X7D, 0XEF, 0XCB, 0X52, 0XB3, 0X94, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0XFF,
    0XE8, 0X31, 0XA3, 0X08, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0XA3, 0X08, 0X49, 0X42,
    0XFF, 0XF7, 0XFF, 0XF7, 0XFF, 0XFF, 0XFF, 0XFF, 0X6A, 0X4A, 0X01, 0X00, 0X72, 0X84, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XF7, 0XDF, 0XEF, 0X09, 0X3A, 0XA3, 0X08, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11,
    0X04, 0X11, 0X05, 0X11, 0X18, 0XBE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XF0, 0X7B, 0X62, 0X00, 0XE8, 0X31, 0XC7, 0X31, 0X41, 0X00, 0X35, 0XA5, 0XFF, 0XFF, 0X5D, 0XEF,
    0X46, 0X21, 0XC4, 0X10, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0XE4, 0X10, 0XA3, 0X08,
    0X76, 0X9D, 0XFF, 0XF7, 0XFF, 0XFF, 0XB7, 0XAD, 0XA3, 0X08, 0XA3, 0X08, 0XC7, 0X31, 0X9E, 0XE7,
    0XFF, 0XF7, 0XFF, 0XF7, 0X76, 0XA5, 0XA3, 0X08, 0XE4, 0X10, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11,
    0X05, 0X11, 0X05, 0X11, 0X35, 0XA5, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XDB, 0XDE, 0XA7, 0X29, 0X83, 0X00, 0XC4, 0X10, 0XC4, 0X10, 0XC4, 0X10, 0X1C, 0XE7, 0X9E, 0XEF,
    0X05, 0X11, 0XE4, 0X10, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X04, 0X19, 0XC4, 0X08,
    0XE5, 0X10, 0XD1, 0X6B, 0XD1, 0X6B, 0XC5, 0X08, 0X64, 0X00, 0XA5, 0X08, 0X43, 0X00, 0X2B, 0X32,
    0X77, 0X9D, 0XB3, 0X84, 0X25, 0X19, 0XC4, 0X10, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11, 0X04, 0X11,
    0X25, 0X19, 0X26, 0X09, 0X35, 0X9D, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XAF, 0X73, 0X62, 0X00, 0X04, 0X19, 0X05, 0X19, 0X82, 0X00, 0X0D, 0X5B, 0X8E, 0X9B,
    0X62, 0X10, 0X05, 0X11, 0X04, 0X11, 0X04, 0X11, 0X04, 0X19, 0XE4, 0X10, 0X85, 0X00, 0X05, 0X11,
    0XC4, 0X39, 0X81, 0X5A, 0X40, 0X7B, 0X22, 0X9C, 0X43, 0XAC, 0X03, 0XA4, 0X83, 0X9B, 0X82, 0X72,
    0X82, 0X49, 0XC2, 0X18, 0XA4, 0X00, 0XC5, 0X00, 0XE4, 0X10, 0X04, 0X19, 0X04, 0X11, 0X05, 0X19,
    0X47, 0X19, 0X67, 0X11, 0XEC, 0X5A, 0XBE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XDF, 0XFF, 0XDB, 0XD6, 0XC4, 0X10, 0XE4, 0X10, 0X04, 0X11, 0X05, 0X11, 0XA4, 0X18, 0X01, 0XC0,
    0X83, 0X88, 0XE4, 0X00, 0X05, 0X19, 0X04, 0X19, 0XC5, 0X08, 0X44, 0X21, 0X43, 0X83, 0X23, 0XD5,
    0X42, 0XFE, 0XE4, 0XFE, 0X27, 0XFF, 0X07, 0XFF, 0XA4, 0XFE, 0X64, 0XFE, 0X03, 0XFE, 0XA3, 0XFD,
    0XE2, 0XFC, 0X42, 0XEC, 0X83, 0XB3, 0X24, 0X62, 0XE5, 0X10, 0XC4, 0X08, 0X04, 0X19, 0X26, 0X19,
    0XA8, 0X19, 0X87, 0X21, 0X00, 0X90, 0XD3, 0XBC, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XF0, 0X7B, 0X42, 0X00, 0X05, 0X19, 0X05, 0X11, 0X83, 0X28, 0X01, 0XD0,
    0X44, 0XF8, 0XA3, 0X48, 0XE4, 0X00, 0XC5, 0X08, 0X44, 0X5A, 0X02, 0XED, 0XE2, 0XFD, 0X02, 0XFE,
    0X66, 0XFE, 0X74, 0XFF, 0XB8, 0XFF, 0X73, 0XFF, 0XE7, 0XF6, 0XA6, 0XF6, 0X45, 0XF6, 0XA4, 0XF5,
    0XC3, 0XFC, 0X62, 0XFC, 0XC2, 0XFC, 0XC2, 0XFC, 0XE3, 0XCB, 0XC4, 0X49, 0X06, 0X11, 0X88, 0X19,
    0X87, 0X01, 0XA4, 0X90, 0X01, 0XF8, 0XEC, 0X9A, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0X9E, 0XF7, 0XE8, 0X31, 0X83, 0X00, 0X05, 0X09, 0X82, 0X40, 0X01, 0XC0,
    0X23, 0XF8, 0X85, 0XF0, 0XA3, 0X48, 0XA4, 0X00, 0X44, 0X5A, 0X02, 0XFD, 0X23, 0XCC, 0XC2, 0XDC,
    0X04, 0XFE, 0X28, 0XFE, 0X48, 0XF6, 0X46, 0XF6, 0X24, 0XF6, 0XE4, 0XF5, 0X64, 0XFD, 0XE3, 0XFC,
    0X62, 0XFC, 0XC2, 0XFC, 0X02, 0XE4, 0X02, 0XDC, 0XE2, 0XFC, 0XA4, 0X7A, 0X48, 0X01, 0X67, 0X01,
    0XC4, 0X78, 0X24, 0XF8, 0X02, 0XF8, 0X84, 0XB0, 0X7D, 0XE7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDB, 0XDE, 0X25, 0X19, 0XA3, 0X00, 0XC4, 0X38, 0X02, 0XE0,
    0X22, 0XD8, 0X44, 0XF8, 0XA6, 0XF8, 0XA4, 0X78, 0X63, 0X00, 0X43, 0X21, 0X83, 0X72, 0X83, 0X39,
    0X82, 0X9B, 0X21, 0XF5, 0X61, 0XFD, 0X22, 0XFD, 0XE2, 0XFC, 0XA2, 0XFC, 0X42, 0XFC, 0X42, 0XFC,
    0X42, 0XFC, 0X22, 0XAB, 0X83, 0X41, 0XC3, 0X92, 0X04, 0X52, 0X26, 0X01, 0X25, 0X19, 0XA4, 0X98,
    0X44, 0XF8, 0X23, 0XF8, 0X02, 0XF8, 0XA4, 0XD0, 0X9E, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X9A, 0XD6, 0X87, 0X29, 0XA5, 0X00, 0X43, 0XB8,
    0X22, 0XF8, 0X23, 0XE0, 0X65, 0XF8, 0XE8, 0XF8, 0X07, 0XC9, 0X83, 0X48, 0X42, 0X00, 0XA3, 0X00,
    0X84, 0X00, 0X63, 0X29, 0XA2, 0X7A, 0X62, 0XB3, 0XA2, 0XCB, 0X62, 0XD3, 0X02, 0XBB, 0X82, 0X8A,
    0X83, 0X39, 0XA4, 0X00, 0XE5, 0X00, 0XE5, 0X00, 0XE5, 0X08, 0XC4, 0X60, 0X64, 0XD8, 0X44, 0XF8,
    0X24, 0XF8, 0X23, 0XF8, 0X02, 0XF8, 0X83, 0X88, 0XDB, 0XC6, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X3D, 0XE7, 0X50, 0X5B, 0X08, 0X31,
    0X23, 0XE8, 0X43, 0XF8, 0X44, 0XF0, 0X65, 0XF8, 0X09, 0XF9, 0XAB, 0XF9, 0X89, 0XD1, 0X06, 0X89,
    0XA3, 0X48, 0X42, 0X18, 0X02, 0X00, 0X42, 0X00, 0X61, 0X00, 0X82, 0X00, 0X62, 0X00, 0X62, 0X00,
    0X83, 0X00, 0XA3, 0X20, 0XC4, 0X50, 0XA5, 0X88, 0X85, 0XD8, 0X65, 0XF8, 0X44, 0XF8, 0X44, 0XF8,
    0X23, 0XF8, 0X23, 0XF8, 0X03, 0XD0, 0X82, 0X10, 0XC7, 0X29, 0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X6C, 0X32,
    0XA5, 0X38, 0X02, 0XD8, 0X23, 0XF8, 0X65, 0XF8, 0X66, 0XF8, 0XA7, 0XF8, 0X4A, 0XF9, 0X0C, 0XFA,
    0X4D, 0XFA, 0X4C, 0XEA, 0X0B, 0XD2, 0XA9, 0XB9, 0X68, 0XB1, 0X47, 0XA9, 0X27, 0XB1, 0X07, 0XB9,
    0X07, 0XD1, 0XE7, 0XE8, 0XC7, 0XF8, 0XA7, 0XF8, 0X65, 0XF8, 0X65, 0XF8, 0X44, 0XF8, 0X23, 0XF8,
    0X03, 0XF8, 0X02, 0XD0, 0XA3, 0X28, 0X05, 0X09, 0XC4, 0X08, 0XEC, 0X5A, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFB, 0XDE, 0X05, 0X19,
    0XC4, 0X00, 0XA7, 0X41, 0XE6, 0XC0, 0X03, 0XF8, 0X86, 0XF8, 0XA7, 0XF8, 0X87, 0XF8, 0X86, 0XF8,
    0XC7, 0XF8, 0X29, 0XF9, 0X8A, 0XF9, 0XAB, 0XF9, 0XAB, 0XF9, 0X8B, 0XF9, 0X6A, 0XF9, 0X29, 0XF9,
    0X08, 0XF9, 0XC7, 0XF8, 0XA6, 0XF8, 0X86, 0XF8, 0X65, 0XF8, 0X64, 0XF8, 0X23, 0XF8, 0X02, 0XF0,
    0X06, 0XB1, 0X25, 0X29, 0XE4, 0X00, 0XE4, 0X10, 0X25, 0X19, 0X25, 0X19, 0X14, 0X9D, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X96, 0XAD, 0X62, 0X00,
    0X82, 0X08, 0X35, 0X95, 0XBA, 0XCE, 0X8B, 0XA2, 0X44, 0XD0, 0X25, 0XF8, 0X87, 0XF8, 0XA7, 0XF8,
    0XC7, 0XF8, 0XA7, 0XF8, 0X87, 0XF8, 0X86, 0XF8, 0X86, 0XF8, 0X86, 0XF8, 0X87, 0XF8, 0XA7, 0XF8,
    0XA7, 0XF8, 0XA6, 0XF8, 0X85, 0XF8, 0X65, 0XF8, 0X64, 0XF8, 0X24, 0XF0, 0X64, 0XB8, 0X0D, 0X93,
    0XBB, 0XB6, 0XCF, 0X63, 0X83, 0X08, 0X04, 0X11, 0XE4, 0X10, 0X66, 0X21, 0X49, 0X3A, 0X5D, 0XEF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XD3, 0X94, 0X42, 0X00,
    0XE4, 0X10, 0XBB, 0XCE, 0XFF, 0XFF, 0XBE, 0XE7, 0X76, 0XB5, 0XCC, 0XAA, 0X07, 0XC1, 0X45, 0XE0,
    0X45, 0XF8, 0X46, 0XF8, 0X66, 0XF8, 0X86, 0XF8, 0X86, 0XF8, 0X86, 0XF8, 0X86, 0XF8, 0X65, 0XF8,
    0X45, 0XF8, 0X65, 0XF8, 0X65, 0XE8, 0X44, 0XD0, 0X43, 0XA8, 0X01, 0X88, 0X82, 0X90, 0X3C, 0XD7,
    0XFF, 0XEF, 0X55, 0X95, 0X83, 0X08, 0X04, 0X11, 0X04, 0X11, 0X05, 0X19, 0X46, 0X19, 0XB3, 0X94,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XB3, 0X94, 0X41, 0X00,
    0X86, 0X21, 0X5D, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XDF, 0XE7, 0X7A, 0XC6, 0XD3, 0XB4,
    0X4E, 0XB3, 0X2A, 0XC2, 0X68, 0XD1, 0XE6, 0XE0, 0XA6, 0XE8, 0XA5, 0XE8, 0XA5, 0XE8, 0XE6, 0XD8,
    0X88, 0XC9, 0X06, 0XA9, 0X22, 0XA8, 0X02, 0XA8, 0X00, 0XA0, 0X00, 0XC8, 0X00, 0XD8, 0XF7, 0XE5,
    0XFF, 0XE7, 0XF8, 0XAD, 0XC4, 0X10, 0XE4, 0X10, 0X04, 0X11, 0XE4, 0X10, 0X05, 0X11, 0X8B, 0X4A,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X55, 0XA5, 0X41, 0X00,
    0XA7, 0X29, 0X5D, 0XDF, 0XFF, 0XF7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XEF, 0X7D, 0XDF, 0XDB, 0XCE, 0X59, 0XCE, 0XF8, 0XCD, 0XD7, 0XCD, 0XF7, 0XC5, 0X79, 0XCE,
    0XFB, 0XBE, 0XAB, 0XA2, 0X03, 0XF0, 0X45, 0XF8, 0X42, 0XD0, 0X43, 0XE8, 0X00, 0XF0, 0X72, 0XD4,
    0XFF, 0XDF, 0X39, 0XAE, 0XE4, 0X10, 0XE4, 0X10, 0X04, 0X11, 0XE4, 0X10, 0X05, 0X11, 0X87, 0X29,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X59, 0XCE, 0X83, 0X08,
    0X46, 0X21, 0X1C, 0XD7, 0XFF, 0XF7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XEF, 0X8B, 0XBA, 0X04, 0XF8, 0X45, 0XF8, 0X62, 0XE0, 0X44, 0XF0, 0X00, 0XF8, 0X8E, 0XDB,
    0XFF, 0XDF, 0XF8, 0XA5, 0XC4, 0X10, 0XE4, 0X10, 0XE4, 0X10, 0X04, 0X11, 0XE4, 0X10, 0X25, 0X19,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XBE, 0XF7, 0X87, 0X29,
    0X83, 0X08, 0X39, 0XB6, 0XFF, 0XF7, 0XDF, 0XF7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XBE, 0XE7, 0X4A, 0XBA, 0X03, 0XF8, 0X45, 0XF8, 0X64, 0XF8, 0X44, 0XF8, 0X00, 0XF8, 0X6E, 0XE3,
    0XFF, 0XD7, 0XF4, 0X8C, 0X83, 0X08, 0X04, 0X11, 0XE4, 0X10, 0XE4, 0X10, 0XE4, 0X10, 0X05, 0X19,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0X73,
    0X00, 0X00, 0X72, 0X84, 0XFF, 0XEF, 0XBE, 0XEF, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XDF, 0XE7, 0X8B, 0XBA, 0X03, 0XF8, 0X45, 0XF8, 0X45, 0XF8, 0X23, 0XF8, 0X00, 0XF8, 0XD3, 0XD4,
    0XFF, 0XD7, 0X4E, 0X5B, 0X21, 0X00, 0X29, 0X3A, 0X55, 0XA5, 0X83, 0X08, 0XC4, 0X10, 0X25, 0X19,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFB, 0XDE,
    0XA3, 0X08, 0XE8, 0X31, 0X9E, 0XDF, 0X9E, 0XE7, 0XBF, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XF7, 0X51, 0XBC, 0X02, 0XE0, 0X03, 0XF8, 0X03, 0XF0, 0X43, 0XE0, 0XEC, 0XC2, 0X7E, 0XCF,
    0XFC, 0XBE, 0X46, 0X21, 0X21, 0X00, 0XD3, 0X94, 0XFF, 0XFF, 0X51, 0X84, 0X00, 0X00, 0X87, 0X29,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X51, 0X84, 0X00, 0X00, 0XF4, 0X8C, 0XFF, 0XEF, 0X9E, 0XE7, 0XBF, 0XEF, 0XDF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0X3D, 0XDF, 0X55, 0XBD, 0X52, 0XBC, 0X72, 0XBC, 0XB7, 0XB5, 0X5D, 0XC7, 0XFF, 0XDF,
    0XF0, 0X6B, 0X00, 0X00, 0X09, 0X3A, 0XBF, 0XF7, 0XFF, 0XFF, 0XFF, 0XFF, 0X14, 0X9D, 0X55, 0XA5,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XAC, 0X4A, 0XA4, 0X08, 0XBB, 0XBE, 0XDF, 0XE7, 0X7E, 0XE7, 0XBE, 0XEF, 0XDF, 0XF7,
    0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XDF, 0XFF, 0XDF, 0XF7, 0XFF, 0XEF, 0XDF, 0XDF, 0XBF, 0XD7, 0X9E, 0XD7, 0XDF, 0XDF, 0XD8, 0XA5,
    0X83, 0X08, 0X26, 0X11, 0XDB, 0XD6, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X79, 0XEE, 0X8B, 0XDC, 0X21, 0X31, 0XA9, 0X21, 0X3D, 0XCF, 0XBF, 0XDF, 0X7E, 0XDF, 0X9E, 0XE7,
    0XBE, 0XEF, 0XBF, 0XEF, 0XDF, 0XF7, 0XDF, 0XF7, 0XDF, 0XF7, 0XDF, 0XF7, 0XDF, 0XF7, 0XBF, 0XEF,
    0XBE, 0XEF, 0X9E, 0XE7, 0X7E, 0XDF, 0X5E, 0XD7, 0X5E, 0XD7, 0XDF, 0XDF, 0X9A, 0XB6, 0X26, 0X19,
    0X42, 0X08, 0XED, 0XA3, 0XBF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X74, 0XDD,
    0XC0, 0XDB, 0X00, 0XFE, 0X42, 0XEE, 0X02, 0X42, 0X89, 0X21, 0X7B, 0XB6, 0XDF, 0XDF, 0X7E, 0XD7,
    0X7E, 0XDF, 0X7E, 0XDF, 0X9E, 0XE7, 0X9E, 0XE7, 0X9E, 0XE7, 0X9E, 0XE7, 0X9E, 0XE7, 0X7E, 0XDF,
    0X7E, 0XDF, 0X5D, 0XD7, 0X5D, 0XD7, 0X9E, 0XDF, 0XFF, 0XE7, 0XF8, 0XA5, 0X07, 0X11, 0XE3, 0X18,
    0X02, 0XC5, 0X60, 0XFD, 0XE6, 0XD3, 0XDB, 0XEE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X9E, 0XF7, 0X84, 0XBA,
    0XC1, 0XFC, 0X42, 0XFE, 0X82, 0XFE, 0XA2, 0XFE, 0X81, 0X83, 0X45, 0X21, 0X74, 0X74, 0X5E, 0XC7,
    0XDF, 0XDF, 0X7E, 0XD7, 0X5E, 0XD7, 0X5D, 0XD7, 0X5E, 0XD7, 0X5E, 0XD7, 0X5D, 0XD7, 0X5D, 0XD7,
    0X5E, 0XD7, 0X9E, 0XDF, 0XFF, 0XE7, 0X3D, 0XC7, 0XF1, 0X63, 0X84, 0X08, 0X42, 0X52, 0X26, 0XE6,
    0X29, 0XFF, 0X86, 0XFE, 0XE0, 0XF3, 0X6A, 0XC3, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X18, 0XDE, 0XC1, 0XD2,
    0XA2, 0XFD, 0X22, 0XFE, 0X42, 0XFE, 0X42, 0XFE, 0X62, 0XFE, 0XE2, 0XD4, 0X41, 0X6A, 0X49, 0X42,
    0X53, 0X74, 0X3B, 0XA6, 0X3E, 0XC7, 0XBF, 0XD7, 0XBF, 0XD7, 0XBF, 0XD7, 0XBF, 0XDF, 0XBF, 0XD7,
    0X3E, 0XC7, 0X1A, 0XA6, 0XF2, 0X63, 0XA7, 0X29, 0X82, 0X41, 0X22, 0XB4, 0X62, 0XFE, 0X83, 0XFE,
    0XAA, 0XFE, 0X0F, 0XFF, 0X67, 0XFD, 0X63, 0XBA, 0X3C, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X9A, 0XE6, 0X80, 0XD2,
    0X21, 0XFD, 0XC2, 0XFD, 0XE2, 0XF5, 0XC2, 0XF5, 0X82, 0XF5, 0X82, 0XFD, 0X62, 0XFD, 0X61, 0XDC,
    0X21, 0X9B, 0X84, 0X6A, 0XE9, 0X6A, 0X2C, 0X63, 0XAF, 0X63, 0X11, 0X74, 0X6E, 0X63, 0X2C, 0X63,
    0X89, 0X5A, 0X04, 0X52, 0X81, 0X7A, 0XC2, 0XCB, 0XE2, 0XFC, 0X62, 0XFD, 0X82, 0XFD, 0XC2, 0XFD,
    0XC2, 0XFD, 0XE4, 0XFD, 0X24, 0XFD, 0X62, 0XCA, 0X1C, 0XE7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XB1, 0XCC,
    0X81, 0XD2, 0XC0, 0XF3, 0XC1, 0XFC, 0X02, 0XFD, 0X02, 0XFD, 0XE2, 0XFC, 0XC2, 0XFC, 0XC2, 0XFC,
    0X81, 0XFC, 0X80, 0XFB, 0XC0, 0XC9, 0XA4, 0X81, 0X35, 0XAD, 0X59, 0XCE, 0X71, 0X9C, 0X21, 0X81,
    0X00, 0XDA, 0XA1, 0XFB, 0X82, 0XFC, 0XA2, 0XFC, 0X82, 0XFC, 0XA2, 0XFC, 0X02, 0XFD, 0X22, 0XFD,
    0XE2, 0XFC, 0X00, 0XFC, 0X60, 0XDA, 0X90, 0XCC, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X5D, 0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0X59, 0XDE, 0X0D, 0XC4, 0X06, 0XCB, 0XE4, 0XD2, 0X03, 0XDB, 0X03, 0XDB, 0XE3, 0XDA, 0XC3, 0XD2,
    0XA4, 0XC2, 0X09, 0XB3, 0XD2, 0XBC, 0X9E, 0XF7, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFB, 0XE6,
    0X0E, 0XB4, 0XA6, 0XBA, 0X83, 0XD2, 0XE3, 0XE2, 0X02, 0XEB, 0X22, 0XEB, 0X22, 0XE3, 0X03, 0XDB,
    0XE4, 0XD2, 0X6A, 0XC3, 0XB6, 0XD5, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
};

extern const uint8_t GB_FON_start[]   asm("_binary_GB_FON_start");
extern const uint8_t GB_FON_end[]     asm("_binary_GB_FON_end");

uint8_t lcd_dc_level = 0;

void lcd_delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_RATE_MS);
}

void lcd_set_dc(uint8_t dc)
{
    lcd_dc_level = dc;
}

//向液晶屏写一个8位指令
void lcd_write_cmd(uint8_t data)
{
    uint32_t buf = data << 24;
    spi_trans_t trans = {0};
    trans.mosi = &buf;
    trans.bits.mosi = 8;

    lcd_set_dc(0);
    spi_trans(HSPI_HOST, trans);
}

//向液晶屏写一个8位数据
void lcd_write_byte(uint8_t data)
{
    uint32_t buf = data << 24;
    spi_trans_t trans = {0};
    trans.mosi = &buf;
    trans.bits.mosi = 8;

    lcd_set_dc(1);
    spi_trans(HSPI_HOST, trans);
}
//向液晶屏写一个16位数据
void lcd_write_2byte(uint16_t data)
{
    uint32_t buf = data << 16;
    spi_trans_t trans = {0};
    trans.mosi = &buf;
    trans.bits.mosi = 16;

    lcd_set_dc(1);
    spi_trans(HSPI_HOST, trans);
}

void lcd_write_word(uint32_t data)
{
    spi_trans_t trans = {0};
    trans.mosi = &data;
    trans.bits.mosi = 32;

    lcd_set_dc(1);
    spi_trans(HSPI_HOST, trans);
}

void lcd_rst()
{
    gpio_set_level(LCD_RST_GPIO, 0);
    lcd_delay_ms(100);
    gpio_set_level(LCD_RST_GPIO, 1);
    lcd_delay_ms(100);
}

void lcd_init()
{
    lcd_rst();//lcd_rst before LCD Init.

    lcd_write_cmd(0x11);//Sleep exit
    lcd_delay_ms(120);
    lcd_write_cmd(0x21);

    lcd_write_cmd(0xB1);
    lcd_write_byte(0x05);
    lcd_write_byte(0x3A);
    lcd_write_byte(0x3A);

    lcd_write_cmd(0xB2);
    lcd_write_byte(0x05);
    lcd_write_byte(0x3A);
    lcd_write_byte(0x3A);

    lcd_write_cmd(0xB3);
    lcd_write_byte(0x05);
    lcd_write_byte(0x3A);
    lcd_write_byte(0x3A);
    lcd_write_byte(0x05);
    lcd_write_byte(0x3A);
    lcd_write_byte(0x3A);

    lcd_write_cmd(0xB4);
    lcd_write_byte(0x03);

    lcd_write_cmd(0xC0);
    lcd_write_byte(0x62);
    lcd_write_byte(0x02);
    lcd_write_byte(0x04);

    lcd_write_cmd(0xC1);
    lcd_write_byte(0xC0);

    lcd_write_cmd(0xC2);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x00);

    lcd_write_cmd(0xC3);
    lcd_write_byte(0x8D);
    lcd_write_byte(0x6A);

    lcd_write_cmd(0xC4);
    lcd_write_byte(0x8D);
    lcd_write_byte(0xEE);

    lcd_write_cmd(0xC5);  /*VCOM*/
    lcd_write_byte(0x0E);

    lcd_write_cmd(0xE0);
    lcd_write_byte(0x10);
    lcd_write_byte(0x0E);
    lcd_write_byte(0x02);
    lcd_write_byte(0x03);
    lcd_write_byte(0x0E);
    lcd_write_byte(0x07);
    lcd_write_byte(0x02);
    lcd_write_byte(0x07);
    lcd_write_byte(0x0A);
    lcd_write_byte(0x12);
    lcd_write_byte(0x27);
    lcd_write_byte(0x37);
    lcd_write_byte(0x00);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x0E);
    lcd_write_byte(0x10);

    lcd_write_cmd(0xE1);
    lcd_write_byte(0x10);
    lcd_write_byte(0x0E);
    lcd_write_byte(0x03);
    lcd_write_byte(0x03);
    lcd_write_byte(0x0F);
    lcd_write_byte(0x06);
    lcd_write_byte(0x02);
    lcd_write_byte(0x08);
    lcd_write_byte(0x0A);
    lcd_write_byte(0x13);
    lcd_write_byte(0x26);
    lcd_write_byte(0x36);
    lcd_write_byte(0x00);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x0E);
    lcd_write_byte(0x10);

    lcd_write_cmd(0x3A);
    lcd_write_byte(0x05);

    lcd_write_cmd(0x36);
    lcd_write_byte(0xC8);

    lcd_write_cmd(0x29);
}

void lcd_set_index(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    lcd_write_cmd(0x2a);    // CASET (2Ah): Column Address Set
    // Must write byte than byte
    lcd_write_byte(0x00);
    lcd_write_byte(x_start + 0x1A);
    lcd_write_byte(0x00);
    lcd_write_byte(x_end + 0x1A);

    lcd_write_cmd(0x2b);    // RASET (2Bh): Row Address Set
    lcd_write_byte(0x00);
    lcd_write_byte(y_start + 1);
    lcd_write_byte(0x00);
    lcd_write_byte(y_end + 1);
    lcd_write_cmd(0x2c);    // RAMWR (2Ch): Memory Write
}


void lcd_draw_point(uint16_t x_start, uint16_t y_start, uint16_t color)
{
    lcd_set_index(x_start, y_start, x_start + 1, y_start + 1);
    lcd_write_2byte(color);
}

//画线函数，使用Bresenham 画线算法
void lcd_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Color)
{
    int dx,             // difference in x's
        dy,             // difference in y's
        dx2,            // dx,dy * 2
        dy2,
        x_inc,          // amount in pixel space to move during drawing
        y_inc,          // amount in pixel space to move during drawing
        error,          // the discriminant i.e. error i.e. decision variable
        index;          // used for looping


    lcd_set_index(x0, y0, x0, y0);
    dx = x1 - x0; //计算x距离
    dy = y1 - y0; //计算y距离

    if (dx >= 0) {
        x_inc = 1;
    } else {
        x_inc = -1;
        dx    = -dx;
    }

    if (dy >= 0) {
        y_inc = 1;
    } else {
        y_inc = -1;
        dy    = -dy;
    }

    dx2 = dx << 1;
    dy2 = dy << 1;

    if (dx > dy) { //x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
        //且线的点数等于x距离，以x轴递增画点
        // initialize error term
        error = dy2 - dx;

        // draw the line
        for (index = 0; index <= dx; index++) { //要画的点数不会超过x距离
            //画点
            lcd_draw_point(x0, y0, Color);

            // test if error has overflowed
            if (error >= 0) { //是否需要增加y坐标值
                error -= dx2;

                // move to next line
                y0 += y_inc; //增加y坐标值
            } // end if error overflowed

            // adjust the error term
            error += dy2;

            // move to the next pixel
            x0 += x_inc; //x坐标值每次画点后都递增1
        } // end for
    } // end if |slope| <= 1
    else { //y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
        //以y轴为递增画点
        // initialize error term
        error = dx2 - dy;

        // draw the line
        for (index = 0; index <= dy; index++) {
            // set the pixel
            lcd_draw_point(x0, y0, Color);

            // test if error overflowed
            if (error >= 0) {
                error -= dy2;

                // move to next line
                x0 += x_inc;
            } // end if error overflowed

            // adjust the error term
            error += dx2;

            // move to the next pixel
            y0 += y_inc;
        } // end for
    } // end else |slope| > 1
}

void lcd_draw_circle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc)
{
    //Bresenham算法
    uint16_t  a, b;
    int c;
    a = 0;
    b = R;
    c = 3 - 2 * R;

    while (a < b) {
        lcd_draw_point(X + a, Y + b, fc); //        7
        lcd_draw_point(X - a, Y + b, fc); //        6
        lcd_draw_point(X + a, Y - b, fc); //        2
        lcd_draw_point(X - a, Y - b, fc); //        3
        lcd_draw_point(X + b, Y + a, fc); //        8
        lcd_draw_point(X - b, Y + a, fc); //        5
        lcd_draw_point(X + b, Y - a, fc); //        1
        lcd_draw_point(X - b, Y - a, fc); //        4

        if (c < 0) {
            c = c + 4 * a + 6;
        } else {
            c = c + 4 * (a - b) + 10;
            b -= 1;
        }

        a += 1;
    }

    if (a == b) {
        lcd_draw_point(X + a, Y + b, fc);
        lcd_draw_point(X + a, Y + b, fc);
        lcd_draw_point(X + a, Y - b, fc);
        lcd_draw_point(X - a, Y - b, fc);
        lcd_draw_point(X + b, Y + a, fc);
        lcd_draw_point(X - b, Y + a, fc);
        lcd_draw_point(X + b, Y - a, fc);
        lcd_draw_point(X - b, Y - a, fc);
    }

}

void lcd_draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc)
{
    lcd_draw_line(x, y, x + w, y, 0xEF7D);
    lcd_draw_line(x + w - 1, y + 1, x + w - 1, y + 1 + h, 0x2965);
    lcd_draw_line(x, y + h, x + w, y + h, 0x2965);
    lcd_draw_line(x, y, x, y + h, 0xEF7D);
    lcd_draw_line(x + 1, y + 1, x + 1 + w - 2, y + 1 + h - 2, bc);
}
void lcd_draw_box2(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode)
{
    if (mode == 0)    {
        lcd_draw_line(x, y, x + w, y, 0xEF7D);
        lcd_draw_line(x + w - 1, y + 1, x + w - 1, y + 1 + h, 0x2965);
        lcd_draw_line(x, y + h, x + w, y + h, 0x2965);
        lcd_draw_line(x, y, x, y + h, 0xEF7D);
    }

    if (mode == 1)    {
        lcd_draw_line(x, y, x + w, y, 0x2965);
        lcd_draw_line(x + w - 1, y + 1, x + w - 1, y + 1 + h, 0xEF7D);
        lcd_draw_line(x, y + h, x + w, y + h, 0xEF7D);
        lcd_draw_line(x, y, x, y + h, 0x2965);
    }

    if (mode == 2)    {
        lcd_draw_line(x, y, x + w, y, 0xffff);
        lcd_draw_line(x + w - 1, y + 1, x + w - 1, y + 1 + h, 0xffff);
        lcd_draw_line(x, y + h, x + w, y + h, 0xffff);
        lcd_draw_line(x, y, x, y + h, 0xffff);
    }
}


/**************************************************************************************
功能描述: 在屏幕显示一凸起的按钮框
输    入: uint16_t x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void lcd_draw_button_down(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_draw_line(x1,  y1,  x2, y1, DGRAY); //H
    lcd_draw_line(x1 + 1, y1 + 1, x2, y1 + 1, LGRAY); //H
    lcd_draw_line(x1,  y1,  x1, y2, DGRAY); //V
    lcd_draw_line(x1 + 1, y1 + 1, x1 + 1, y2, LGRAY); //V
    lcd_draw_line(x1,  y2,  x2, y2, WHITE); //H
    lcd_draw_line(x2,  y1,  x2, y2, WHITE); //V
}

/**************************************************************************************
功能描述: 在屏幕显示一凹下的按钮框
输    入: uint16_t x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void lcd_draw_button_up(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_draw_line(x1,  y1,  x2, y1, WHITE); //H
    lcd_draw_line(x1,  y1,  x1, y2, WHITE); //V

    lcd_draw_line(x1 + 1, y2 - 1, x2, y2 - 1, LGRAY); //H
    lcd_draw_line(x1,  y2,  x2, y2, DGRAY); //H
    lcd_draw_line(x2 - 1, y1 + 1, x2 - 1, y2, LGRAY); //V
    lcd_draw_line(x2, y1, x2, y2, DGRAY);  //V
}

void lcd_clear(uint16_t color)
{
    uint32_t data[16];
    uint32_t i;
    spi_trans_t trans = {0};
    trans.mosi = data;
    trans.bits.mosi = 32 * 16;

    for (i = 0; i < 16; i++) {
        data[i] = (color << 16) | color;
    }

    lcd_set_index(0, 0, 80 - 1, 160 - 1);
    lcd_set_dc(1);

    for (i = 0; i < 400; i++) {
        spi_trans(HSPI_HOST, trans);
    }
}

//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void lcd_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint16_t i, j;
    lcd_set_index(xsta, ysta, xend, yend);

    for (i = ysta; i <= yend; i++) {
        for (j = xsta; j <= xend; j++) {
            lcd_write_2byte(color);
        }
    }
}

//取模方式 水平扫描 从左到右 低位在前
void lcd_show_qq(const uint8_t *p) //显示40*40 QQ图片
{
    int x, y, z;
    uint32_t data_buf[20];
    spi_trans_t trans = {0};
    lcd_set_index(0, 0, 80 - 1, 160 - 1); // 按顺序填充数据，能够最大利用带宽
    lcd_set_dc(1);

    for (z = 0; z < 4; z++) {
        for (y = 0; y < 40; y++) {
            for (x = 0; x < 20; x++) {
                data_buf[x] = *(p + y * 80 + x * 4 + 1) << 24 | *(p + y * 80 + x * 4) << 16 | *(p + y * 80 + x * 4 + 3) << 8 | *(p + y * 80 + x * 4 + 2);
            }

            for (x = 0; x < 2; x++) {
                trans.mosi = data_buf;
                trans.bits.mosi = 32 * 16;
                spi_trans(HSPI_HOST, trans);
                trans.mosi = data_buf + 16;
                trans.bits.mosi = 32 * 4;
                spi_trans(HSPI_HOST, trans);
            }
        }
    }
}

void IRAM_ATTR spi_event_callback(int event, void *arg)
{
    switch (event) {
        case SPI_INIT_EVENT: {

        }
        break;

        case SPI_TRANS_START_EVENT: {
            gpio_set_level(LCD_DC_GPIO, lcd_dc_level);
        }
        break;

        case SPI_TRANS_DONE_EVENT: {
        }
        break;

        case SPI_DEINIT_EVENT: {
        }
        break;
    }
}

void app_main(void)
{
    uint16_t test_color[16] = {BLACK, NAVY, DGREEN, DCYAN, MAROON, PURPLE, OLIVE, LGRAY, DGRAY, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE};
    int x = 0;
    ESP_LOGI(TAG, "init hspi");

    rtc_clk_cpu_freq_set(RTC_CPU_FREQ_160M);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = LCD_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Cancel hardware cs
    spi_config.interface.cs_en = 0;
    spi_config.interface.miso_en = 0;
    spi_config.interface.cpol = 1;
    spi_config.interface.cpha = 1;
    // Set SPI to master mode
    // 8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_40MHz_DIV;
    // Register SPI event callback function
    spi_config.event_cb = spi_event_callback;
    spi_init(HSPI_HOST, &spi_config);

    lcd_init();
    lcd_clear(BLACK);
    lcd_draw_point(5-1,5-1,YELLOW);
    lcd_fill(10-1,10-1,30-1,30-1,RED);
    lcd_fill(60-1,140-1,70-1,150-1,CYAN);
    lcd_draw_circle(40-1, 80-1, 40-1, YELLOW);
    lcd_draw_line(0,0,80-1,160-1,MAGENTA);
    vTaskDelay(2000 / portTICK_RATE_MS);
    //lcd_draw_box(10-1, 10-1, 10, 10, 10);
    // lcd_draw_box2(10-1, 10-1, 20, 20, 3);
    // lcd_draw_button_down(10 - 1, 10 - 1, 20, 20);
    while(1) {
        lcd_clear(test_color[x]);
        vTaskDelay(1000 / portTICK_RATE_MS);
        x++;
        if (x == 16) {
            lcd_show_qq(gImage_qq_logo);
            vTaskDelay(1000 / portTICK_RATE_MS);
            x = 0;
        }
    }
    //lcd_show_qq(gImage_qq_logo);
}


