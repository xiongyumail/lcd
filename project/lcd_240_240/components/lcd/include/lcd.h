#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif 

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

void lcd_write_data(uint16_t *data, size_t len);

void lcd_rst();

void lcd_init();

#ifdef __cplusplus
}
#endif