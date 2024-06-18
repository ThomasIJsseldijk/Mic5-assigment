#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ssd1306.h"
#include "queue.h"
#include "string.h"

#define i2c_addr 0x3C

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

void vControllerTask(void *pvParameters);

void vJoystickTask(void *pvParameters);

void vHeartbeatTask(void *pvParameters);

void vOledTask(void *pvParameters);

void vTransmitTask(void *pvParameters);

void vReceiveTask(void *pvParameters);

void uart_rx_irq_handler();

typedef struct {
    uint16_t x;
    uint16_t y;
} JoystickData;

typedef struct {
    int8_t x;
    int8_t y;
    uint16_t temp;
    uint16_t speed;
    uint16_t fuel;
} OledData;

typedef struct {
    int8_t x;
    int8_t y;
} transmitData;

typedef struct {
    uint16_t temp;
    uint16_t speed;
    uint16_t fuel;
} receiveData;

QueueHandle_t xJoystickQueue = NULL;
QueueHandle_t xOledQueue = NULL;
QueueHandle_t xTransmitQueue = NULL;
QueueHandle_t xReceiveQueue = NULL;
QueueHandle_t xUARTQueue = NULL;

TaskHandle_t xHandleControllerTask = NULL;
TaskHandle_t xHandleJoystickTask = NULL;
TaskHandle_t xHandleHeartbeatTask = NULL;
TaskHandle_t xHandleOledTask = NULL;
TaskHandle_t xHandleTransmitTask = NULL;
TaskHandle_t xHandleReceiveTask = NULL;

int main() {
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    irq_set_exclusive_handler(UART0_IRQ, uart_rx_irq_handler);
    irq_set_enabled(UART0_IRQ, true);

    uart_set_irq_enables(UART_ID, true, false);

    xJoystickQueue = xQueueCreate(1, sizeof(JoystickData));
    xOledQueue = xQueueCreate(10, sizeof(OledData));
    xTransmitQueue = xQueueCreate(10, sizeof(transmitData));
    xReceiveQueue = xQueueCreate(10, sizeof(receiveData));
    xUARTQueue = xQueueCreate(100, sizeof(char));

    xTaskCreate(vControllerTask, "Controller Task", 256, NULL, 3, &xHandleControllerTask);  // Priority: 3
    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 2, &xHandleJoystickTask);       // Priority: 2
    xTaskCreate(vHeartbeatTask, "Heartbeat Task", 256, NULL, 1, &xHandleHeartbeatTask);    // Priority: 1
    xTaskCreate(vOledTask, "OLED Task", 256, NULL, 2, &xHandleOledTask);                  // Priority: 2
    xTaskCreate(vTransmitTask, "Transmit Task", 256, NULL, 2, &xHandleTransmitTask);      // Priority: 2
    xTaskCreate(vReceiveTask, "Receive Task", 256, NULL, 3, &xHandleReceiveTask);         // Priority: 3


    vTaskStartScheduler();

    while (1) {
        tight_loop_contents();
    }

    return 0;
}

void vControllerTask(void *pvParameters) {
    JoystickData joystickData;
    OledData oledData;
    transmitData transmitdata;
    receiveData receivedata;

    while (1) {
        if (xQueueReceive(xJoystickQueue, &joystickData, pdMS_TO_TICKS(5)) == pdTRUE) {
            oledData.x = (int8_t) ((joystickData.x - 2048) / 100);
            oledData.y = (int8_t) ((joystickData.y - 2048) / 100);

            xQueueSend(xOledQueue, &oledData, 0);

            transmitdata.x = (joystickData.x - 2048) / 16;
            transmitdata.y = (joystickData.y - 2048) / 16;

            xQueueSend(xTransmitQueue, &transmitdata, 0);
        }
        if (xQueueReceive(xReceiveQueue, &receivedata, pdMS_TO_TICKS(5)) == pdTRUE) {
            oledData.temp = receivedata.temp;
            oledData.speed = receivedata.speed;
            oledData.fuel = receivedata.fuel;

            xQueueSend(xOledQueue, &oledData, 0);
        }
    }
}

void vJoystickTask(void *pvParameters) {
    JoystickData joystickData;

    adc_init();
    adc_gpio_init(26); // X-axis
    adc_gpio_init(27); // Y-axis

    while (1) {
        adc_select_input(0);
        joystickData.x = adc_read();
        adc_select_input(1);
        joystickData.y = adc_read();

        xQueueSend(xJoystickQueue, &joystickData, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100)); // Un-commented delay
    }
}

void vHeartbeatTask(void *pvParameters) {
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    while (1) {
        gpio_put(25, !gpio_get(25)); // Toggle LED on Pico board
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vOledTask(void *pvParameters) {
    OledData oledData;

    i2c_init(i2c0, 400 * 1000); // 400 kHz
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    ssd1306_init(i2c0, i2c_addr);
    ssd1306_clear();

    while (1) {
        xQueueReceive(xOledQueue, &oledData, portMAX_DELAY);

        ssd1306_clear();
        ssd1306_draw_circle(96, 32, 30);
        ssd1306_draw_filled_circle(96 + oledData.x, 32 + oledData.y, 5);
        char buffer[20];
        snprintf(buffer, 20, "%d C", oledData.temp);
        ssd1306_draw_text(0, 0, "temp");
        ssd1306_draw_text(0, 8, buffer);
        snprintf(buffer, 20, "%d km/h", oledData.speed);
        ssd1306_draw_text(0, 20, "speed");
        ssd1306_draw_text(0, 28, buffer);
        snprintf(buffer, 20, "%d%%", oledData.fuel);
        ssd1306_draw_text(0, 40, "fuel");
        ssd1306_draw_text(0, 48, buffer);
        ssd1306_display();
    }
}

void vTransmitTask(void *pvParameters) {
    transmitData transmitdata;

    while (1) {
        if (xQueueReceive(xTransmitQueue, &transmitdata, 0) == pdTRUE) {
            char buffer[20];
            snprintf(buffer, 20, "X%d,Y%d\n", transmitdata.x, transmitdata.y);
            uart_puts(UART_ID, buffer);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


// Receive task
// reads data from UART until a '\0' is received
// parses the data and sends it to the receive queue
void vReceiveTask(void *pvParameters) {
    receiveData receivedata;
    char buffer[50];  // Increased buffer size to 50 to handle maximum expected input
    int i = 0;
    char c;

    while (1) {
        if (xQueueReceive(xUARTQueue, &c, portMAX_DELAY) == pdTRUE) {
            if (c == '\n' || c == '\r') {  // Check for newline or carriage return as message terminator
                buffer[i] = '\0';  // Null-terminate the string
                sscanf(buffer, "%hu,%hu,%hu", &receivedata.temp, &receivedata.speed, &receivedata.fuel);
                xQueueSend(xReceiveQueue, &receivedata, portMAX_DELAY);
                printf("Received: %hu, %hu, %hu\n", receivedata.temp, receivedata.speed, receivedata.fuel);
                i = 0;
            } else {
                buffer[i] = c;
                i++;
                if (i >= sizeof(buffer)) {  // Prevent buffer overflow
                    i = 0;  // Reset index if buffer overflows
                }
            }
        }
    }
}

void uart_rx_irq_handler() {
    irq_clear(UART0_IRQ);
    char c = uart_getc(UART_ID);
    xQueueSendFromISR(xUARTQueue, &c, NULL);
}