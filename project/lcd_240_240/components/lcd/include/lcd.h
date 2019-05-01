#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define LCD_SCL_GPIO    14
#define LCD_SDA_GPIO    13
#define LCD_DC_GPIO     12
#define LCD_RST_GPIO    15
#define LCD_PIN_SEL     (1ULL<<LCD_DC_GPIO) | (1ULL<<LCD_RST_GPIO) 

void inline lcd_delay_ms(uint32_t time)
{
    vTaskDelay(time /portTICK_RATE_MS);
}

void inline lcd_set_res(uint8_t state)
{
    gpio_set_level(LCD_RST_GPIO, state);
}

void inline lcd_set_dc(uint8_t state)
{
    gpio_set_level(LCD_DC_GPIO, state);
}

void inline lcd_set_cs(uint8_t state)
{

}

void inline lcd_set_blk(uint8_t state)
{

}

void lcd_set_index(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end);

void lcd_write_data(uint32_t *data, uint32_t len);

void lcd_clear(uint16_t color);

void lcd_rst();

void lcd_init();

#ifdef __cplusplus
}
#endif