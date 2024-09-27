#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define GET_BANNER_TX (GPIO_NUM_1)
#define GET_BANNER_RX (GPIO_NUM_3)

#define DISPLAY_BANNER_TX (GPIO_NUM_4)
#define DISPLAY_BANNER_RX (GPIO_NUM_5)

#define GET_BANNER_UART      UART_NUM_0
#define SEND_BANNER_UART     UART_NUM_1
#define ECHO_UART_BAUD_RATE 115200
#define ECHO_TASK_STACK_SIZE 1024


#define BUF_SIZE (128)
#define PAN_SIZE (25)


uint8_t *banner;
uint8_t banner_index = 0;

static void get_banner(void *arg)
{

    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

    uart_driver_install(GET_BANNER_UART, BUF_SIZE , 0, 0, NULL, intr_alloc_flags);
    uart_param_config(GET_BANNER_UART, &uart_config);
    ESP_ERROR_CHECK(uart_set_pin(GET_BANNER_UART, GET_BANNER_TX, GET_BANNER_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    uart_driver_install(SEND_BANNER_UART, BUF_SIZE , 0, 0, NULL, intr_alloc_flags);
    uart_param_config(SEND_BANNER_UART, &uart_config);
    uart_set_pin(SEND_BANNER_UART, DISPLAY_BANNER_TX, DISPLAY_BANNER_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);


    // Configure a temporary buffer for the incoming data
    banner = (uint8_t *) malloc(PAN_SIZE);
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    banner[25] = '\0';
    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(GET_BANNER_UART, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        
        for (int len_cnt = 0; len_cnt < len; len_cnt++) {
            uint8_t current_char = data[len_cnt];
            switch(current_char) {
                case 8:
                    if (banner_index > 0) {
                        banner_index -= 1;
                    }
                    uart_write_bytes(GET_BANNER_UART, (const char *) "\b \b", 3);
                    banner[banner_index] = ' ';
                    break;
                case 13:
                    uart_write_bytes(GET_BANNER_UART, (const char *) "\033[H\033[J", 6);
                    uart_write_bytes(GET_BANNER_UART, (const char *) "Pancarta enviada.", 18);
                    uart_write_bytes(SEND_BANNER_UART, (const char *) banner, banner_index);
                    break;
                default: 
                    if(((current_char >= 'a' && current_char <= 'z') || 
                        (current_char >= 'A' && current_char <= 'Z') || 
                        (current_char >= '0' && current_char <= '9') ||
                        (current_char == '!' || current_char == '.' || current_char == ',' || current_char == '-' || current_char == ' ')) && 
                        (banner_index < PAN_SIZE) ) {
                        banner[banner_index] = data[len_cnt];
                        banner_index += 1;
                        char temp[2] = { current_char, '\0' };
                        uart_write_bytes(GET_BANNER_UART, (const char *) temp, 2);
                    }
                    break;
            }
        }
        
    }
}

void app_main(void)
{
    xTaskCreate(get_banner, "get_banner_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
