#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/i2c.h"

#define SSD1306_I2C_ADDR 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

void ssd1306_init(i2c_inst_t *i2c, uint8_t address);
void ssd1306_send_command(uint8_t command);
void ssd1306_clear();
void ssd1306_draw_pixel(int x, int y);
void ssd1306_display();
void ssd1306_draw_text(int x, int y, const char *text);
void ssd1306_draw_circle(int x0, int y0, int radius);
void ssd1306_draw_filled_circle(int x0, int y0, int radius);


#endif
