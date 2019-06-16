#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_libc.h"
#include "driver/gpio.h"
#include "driver/spi.h"
#include "esp8266/gpio_struct.h"
#include "esp8266/spi_struct.h"
#include "lcd.h"

static SemaphoreHandle_t lcd_write_mux = NULL;
uint8_t lcd_dc_state = 0;

#define SPI_BURST_MAX_LEN 64

static void spi_write_data(uint16_t *data, size_t len)
{
    if (len <= 0) {
        return;
    }
    int x, y;
    uint32_t buf[SPI_BURST_MAX_LEN / 4];
    spi_trans_t trans = {0};
    trans.mosi = buf;
    trans.bits.mosi = SPI_BURST_MAX_LEN * 8;
    for (y = 0; y < len / SPI_BURST_MAX_LEN; y++) {
        for (x = 0; x < SPI_BURST_MAX_LEN / 4; x++) {
            buf[x] = (data[x*2] << 16) | data[x*2 + 1];
        }
        spi_trans(HSPI_HOST, trans);
        data += SPI_BURST_MAX_LEN / 2;
    }
    if (len % SPI_BURST_MAX_LEN) {
        trans.bits.mosi = (len % SPI_BURST_MAX_LEN) * 8;
        for (x = 0; x < (len % SPI_BURST_MAX_LEN) / 4; x++) {
            buf[x] = (data[x*2] << 16) | data[x*2 + 1];
        }
        if ((len % SPI_BURST_MAX_LEN) % 4) {
            if ((len % SPI_BURST_MAX_LEN) / 2) {
                buf[x] = (data[x*2] << 24) | (data[x*2 + 1] << 8);
            } else {
                buf[x] = data[x*2] << 24;
            }
        }
        spi_trans(HSPI_HOST, trans);
    }
}

static void lcd_write_cmd(uint8_t data)
{
    xSemaphoreTake(lcd_write_mux, portMAX_DELAY);
    lcd_dc_state = 0;
    spi_write_data((uint16_t *)&data, 1);
    xSemaphoreGive(lcd_write_mux);
}

static void lcd_write_byte(uint8_t data)
{
    xSemaphoreTake(lcd_write_mux, portMAX_DELAY);
    lcd_dc_state = 1;
    spi_write_data((uint16_t *)&data, 1);
    xSemaphoreGive(lcd_write_mux);
}

void lcd_write_data(uint16_t *data, size_t len)
{
    xSemaphoreTake(lcd_write_mux, portMAX_DELAY);
    lcd_dc_state = 1;
    spi_write_data(data, len);
    xSemaphoreGive(lcd_write_mux);
}
    
void lcd_rst()
{
    lcd_set_res(0);
    lcd_delay_ms(100);
    lcd_set_res(1);
    lcd_delay_ms(100);
}

void lcd_set_index(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end)
{    
    lcd_write_cmd(0x2a);    // CASET (2Ah): Column Address Set 
    // Must write byte than byte
    lcd_write_byte(0x00);
    lcd_write_byte(x_start);
    lcd_write_byte(0x00);
    lcd_write_byte(x_end);

    lcd_write_cmd(0x2b);    // RASET (2Bh): Row Address Set 
    lcd_write_byte(0x00);
    lcd_write_byte(y_start);
    lcd_write_byte(0x00);
    lcd_write_byte(y_end);    
    lcd_write_cmd(0x2c);    // RAMWR (2Ch): Memory Write 
}

static void lcd_config()
{    
    lcd_rst();//lcd_rst before LCD Init.
    //    lcd_write_cmd(0x11);//Sleep exit 
    lcd_delay_ms (120);
    lcd_write_cmd(0x36);    // MADCTL (36h): Memory Data Access Control .
    lcd_write_byte(0x00);

    lcd_write_cmd(0x3A);    // COLMOD (3Ah): Interface Pixel Format 
    lcd_write_byte(0x05);

    lcd_write_cmd(0xB2);    // PORCTRL (B2h): Porch Setting 
    lcd_write_byte(0x0C);
    lcd_write_byte(0x0C);
    lcd_write_byte(0x00);
    lcd_write_byte(0x33);
    lcd_write_byte(0x33);

    lcd_write_cmd(0xB7);    // GCTRL (B7h): Gate Control 
    lcd_write_byte(0x35);  

    lcd_write_cmd(0xBB);    // VCOMS (BBh): VCOM Setting 
    lcd_write_byte(0x37);   // 1.415

    lcd_write_cmd(0xC0);    // LCMCTRL (C0h): LCM Control 
    lcd_write_byte(0x2C);

    lcd_write_cmd(0xC2);    // VDVVRHEN (C2h): VDV and VRH Command Enable 
    lcd_write_byte(0x01);

    lcd_write_cmd(0xC3);    // VRHS (C3h): VRH Set 
    lcd_write_byte(0x12);   

    lcd_write_cmd(0xC4);    // VDVS (C4h): VDV Set 
    lcd_write_byte(0x20);  

    lcd_write_cmd(0xC6);    // FRCTRL2 (C6h): Frame Rate Control in Normal Mode 
    lcd_write_byte(0x0F);    

    lcd_write_cmd(0xD0);    // PWCTRL1 (D0h): Power Control 1 
    lcd_write_byte(0xA4);
    lcd_write_byte(0xA1);

    lcd_write_cmd(0xE0);    // PVGAMCTRL (E0h): Positive Voltage Gamma Control 
    lcd_write_byte(0xD0);
    lcd_write_byte(0x04);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x11);
    lcd_write_byte(0x13);
    lcd_write_byte(0x2B);
    lcd_write_byte(0x3F);
    lcd_write_byte(0x54);
    lcd_write_byte(0x4C);
    lcd_write_byte(0x18);
    lcd_write_byte(0x0D);
    lcd_write_byte(0x0B);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x23);

    lcd_write_cmd(0xE1);    // NVGAMCTRL (E1h): Negative Voltage Gamma Control
    lcd_write_byte(0xD0);
    lcd_write_byte(0x04);
    lcd_write_byte(0x0C);
    lcd_write_byte(0x11);
    lcd_write_byte(0x13);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x3F);
    lcd_write_byte(0x44);
    lcd_write_byte(0x51);
    lcd_write_byte(0x2F);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x1F);
    lcd_write_byte(0x20);
    lcd_write_byte(0x23);

    lcd_write_cmd(0x21);    // INVON (21h): Display Inversion On 

    lcd_write_cmd(0x11);    // SLPOUT (11h): Sleep Out 


    lcd_write_cmd(0x29);    // DISPON (29h): Display On
}

static void IRAM_ATTR spi_event_callback(int event, void *arg)
{
    switch (event) {
        case SPI_INIT_EVENT: {

        }
        break;

        case SPI_TRANS_START_EVENT: {
            lcd_set_dc(lcd_dc_state);
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

void lcd_init()
{
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

    lcd_write_mux = xSemaphoreCreateMutex();

    lcd_config();
}