#include "ssd1306.h"
#include "hardware/i2c.h"
#include <math.h>
#include "fonts.h"

i2c_inst_t *ssd1306_i2c;
uint8_t ssd1306_address;

static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

void ssd1306_init(i2c_inst_t *i2c, uint8_t address) {

    ssd1306_i2c = i2c;
    ssd1306_address = address;

    uint8_t init_sequence[] = {
            0xAE,             // Display OFF
            0xD5, 0x80,       // Set Display Clock Divide Ratio/Oscillator Frequency
            0xA8, 0x3F,       // Set Multiplex Ratio
            0xD3, 0x00,       // Set Display Offset
            0x40,             // Set Display Start Line
            0x8D, 0x14,       // Charge Pump Setting
            0x20, 0x00,       // Set Memory Addressing Mode
            0xA1,             // Set Segment Re-map
            0xC8,             // Set COM Output Scan Direction
            0xDA, 0x12,       // Set COM Pins Hardware Configuration
            0x81, 0xCF,       // Set Contrast Control
            0xD9, 0xF1,       // Set Pre-charge Period
            0xDB, 0x40,       // Set VCOMH Deselect Level
            0xA4,             // Entire Display ON
            0xA6,             // Set Normal/Inverse Display
            0xAF              // Display ON
    };

    for (size_t i = 0; i < sizeof(init_sequence); ++i) {
        ssd1306_send_command(init_sequence[i]);
    }
}

void ssd1306_send_command(uint8_t command) {
    uint8_t buffer[2] = {0x00, command};
    i2c_write_blocking(ssd1306_i2c, ssd1306_address, buffer, 2, false);
}

void ssd1306_clear() {
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = 0x00;
    }
}

void ssd1306_draw_pixel(int x, int y) {
    if (x >= 0 && x < SSD1306_WIDTH && y >= 0 && y < SSD1306_HEIGHT) {
        buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    }
}

void ssd1306_display() {
    uint8_t pageAddr[] = {0x00, 0x10, 0xB0};
    for (int page = 0; page < 8; page++) {
        ssd1306_send_command(pageAddr[2] + page);
        ssd1306_send_command(pageAddr[0]);
        ssd1306_send_command(pageAddr[1]);
        uint8_t data[SSD1306_WIDTH + 1];
        data[0] = 0x40;
        for (int i = 0; i < SSD1306_WIDTH; i++) {
            data[i + 1] = buffer[i + page * SSD1306_WIDTH];
        }
        i2c_write_blocking(ssd1306_i2c, ssd1306_address, data, sizeof(data), false);
    }
}

void ssd1306_draw_text(int x, int y, const char *text) {
    int current_x = x;
    int current_y = y;

    while (*text) {
        // Remove leading spaces at the beginning of each line
        if (current_x == x && *text == ' ') {
            text++;
            continue;
        }

        if (*text >= 32 && *text <= 127) {
            for (int i = 0; i < 5; i++) {
                uint8_t line = font[*text - 32][i];
                for (int j = 0; j < 8; j++) {
                    if (line & 1) {
                        ssd1306_draw_pixel(current_x + i, current_y + j);
                    }
                    line >>= 1;
                }
            }
        }

        // Move to the next character position (5 pixels + 1 pixel space)
        current_x += 6;

        // Check if the next character exceeds the display width
        if (current_x + 5 >= SSD1306_WIDTH) {
            // Wrap around to the next line
            current_x = x;
            current_y += 9; // Move to the next row with an extra pixel buffer
        }

        text++;
    }
}

void ssd1306_draw_circle(int x0, int y0, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Draw pixels in all 8 octants using symmetry
        if (y0 + x >= 0 && y0 + x < SSD1306_HEIGHT) {
            ssd1306_draw_pixel(x0 + x, y0 + y);
            ssd1306_draw_pixel(x0 + x, y0 - y);
        }
        if (y0 + y >= 0 && y0 + y < SSD1306_HEIGHT) {
            ssd1306_draw_pixel(x0 + y, y0 + x);
            ssd1306_draw_pixel(x0 + y, y0 - x);
        }
        if (y0 - y >= 0 && y0 - y < SSD1306_HEIGHT) {
            ssd1306_draw_pixel(x0 - y, y0 + x);
            ssd1306_draw_pixel(x0 - y, y0 - x);
        }
        if (y0 - x >= 0 && y0 - x < SSD1306_HEIGHT) {
            ssd1306_draw_pixel(x0 - x, y0 + y);
            ssd1306_draw_pixel(x0 - x, y0 - y);
        }

        y += 1;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x -= 1;
            err += 1 - 2 * x;
        }
    }
}

void ssd1306_draw_filled_circle(int x0, int y0, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Draw lines from (-x, y) to (x, y)
        for (int i = -x; i <= x; ++i) {
            if (y0 + y >= 0 && y0 + y < SSD1306_HEIGHT) {
                ssd1306_draw_pixel(x0 + i, y0 + y);
            }
            if (y0 - y >= 0 && y0 - y < SSD1306_HEIGHT) {
                ssd1306_draw_pixel(x0 + i, y0 - y);
            }
        }

        // Draw lines from (-y, x) to (y, x)
        for (int i = -y; i <= y; ++i) {
            if (y0 + x >= 0 && y0 + x < SSD1306_HEIGHT) {
                ssd1306_draw_pixel(x0 + i, y0 + x);
            }
            if (y0 - x >= 0 && y0 - x < SSD1306_HEIGHT) {
                ssd1306_draw_pixel(x0 + i, y0 - x);
            }
        }

        y += 1;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x -= 1;
            err += 1 - 2 * x;
        }
    }
}