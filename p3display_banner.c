#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define DISPLAY_BANNER_TX (GPIO_NUM_1)
#define DISPLAY_BANNER_RX (GPIO_NUM_3)

#define GET_BANNER_RX (GPIO_NUM_4)
#define GET_BANNER_TX (GPIO_NUM_5)

#define DISPLAY_BANNER_UART UART_NUM_0
#define GET_BANNER_UART UART_NUM_1
#define ECHO_UART_BAUD_RATE 115200
#define ECHO_TASK_STACK_SIZE 1024

#define BUF_SIZE (1024)

// Diccionario para letras y caracteres
const char * ascii_art[][7] = {
    {"A" , "  ##  ", " #  # ", "######", "#    #", "#    #", "#    #"},
    {"B" , "      ", "##### ", "#    #", "##### ", "#    #", "##### "},
    {"C" , " #### ", "#    #", "#     ", "#     ", "#    #", " #### "},
    {"D" , "##### ", "#    #", "#    #", "#    #", "#    #", "##### "},
    {"E" , "      ", "######", "#     ", "##### ", "#     ", "######"},
    {"F" , "######", "#     ", "##### ", "#     ", "#     ", "#     "},
    {"G" , " #### ", "#     ", "#     ", "#  ###", "#    #", " #### "},
    {"H" , "#    #", "#    #", "######", "#    #", "#    #", "#    #"},
    {"I" , "##### ", "  #   ", "  #   ", "  #   ", "  #   ", "##### "},
    {"J" , "######", "   #  ", "   #  ", "   #  ", "#  #  ", "####  "},
    {"K" , "#   # ", "#  #  ", "###   ", "#  #  ", "#   # ", "#    #"},
    {"L" , "#     ", "#     ", "#     ", "#     ", "#     ", "######"},
    {"M" , "#    #", "##  ##", "# ## #", "#    #", "#    #", "#    #"},
    {"N" , "#    #", "##   #", "# #  #", "#  # #", "#   ##", "#    #"},
    {"O" , " #### ", "#    #", "#    #", "#    #", "#    #", " #### "},
    {"P" , "##### ", "#    #", "##### ", "#     ", "#     ", "#     "},
    {"Q" , " #### ", "#    #", "#    #", "#  # #", " #### ", "     #"},
    {"R" , "##### ", "#    #", "##### ", "#  #  ", "#   # ", "#    #"},
    {"S" , "      ", " #### ", "#     ", " #### ", "     #", " #### "},
    {"T" , "##### ", "  #   ", "  #   ", "  #   ", "  #   ", "  #   "},
    {"U" , "#    #", "#    #", "#    #", "#    #", "#    #", " #### "},
    {"V" , "#    #", "#    #", "#    #", "#    #", " #  # ", "  ##  "},
    {"W" , "#    #", "#    #", "#    #", "# ## #", "##  ##", "#    #"},
    {"X" , "#    #", " #  # ", "  ##  ", "  ##  ", " #  # ", "#    #"},
    {"Y" , "#    #", " #  # ", "  ##  ", "  ##  ", "  ##  ", "  ##  "},
    {"Z" , "      ", "######", "    # ", "   #  ", "  #   ", "######"},
    {"a" , "      ", " #### ", "     #", " #####", " #   #", " #####"},
    {"b" , "#     ", "#     ", "#     ", "##### ", "#    #", "##### "},
    {"c" , "      ", "######", "#     ", "#     ", "#     ", "######"},
    {"d" , "     #", "     #", "     #", " #####", "#    #", " #####"},
    {"e" , "      ", " #### ", "#    #", "######", "#     ", " #### "},
    {"f" , "   ###", "  #   ", "##### ", "  #   ", "  #   ", "  #   "},
    {"g" , "      ", " #####", "#    #", " #####", "     #", " #####"},
    {"h" , "#     ", "#     ", "#     ", "##### ", "#    #", "#    #"},
    {"i" , "      ", "   #  ", "      ", "   #  ", "   #  ", "   #  "},
    {"j" , "      ", "   #  ", "      ", "   #  ", "   #  ", "####   "},
    {"k" , "      ", "#   # ", "#  #  ", "###   ", "#  #  ", "#   # "},
    {"l" , "   #  ", "   #  ", "   #  ", "   #  ", "   #  ", "   #  "},
    {"m" , "      ", "      ", "## ## ", "# # # ", "#   # ", "#   # "},
    {"n" , "      ", "      ", "##### ", "#    #", "#    #", "#    #"},
    {"o" , "      ", "      ", " #### ", "#    #", "#    #", " #### "},
    {"p" , "      ", "##### ", "#    #", "##### ", "#     ", "#     "},
    {"q" , "      ", " #####", "#    #", " #####", "     #", "     #"},
    {"r" , "      ", "# ### ", "##    ", "#     ", "#     ", "#     "},
    {"s" , "      ", " #####", "#     ", " #### ", "     #", "##### "},
    {"t" , "  #   ", "  #   ", "##### ", "  #   ", "  #   ", "   ## "},
    {"u" , "      ", "#    #", "#    #", "#    #", "#    #", " #### "},
    {"v" , "      ", "#    #", "#    #", "#    #", " #  # ", "  ##  "},
    {"w" , "      ", "      ", "#     #", "#     #", "#  #  #", " ## ##"},
    {"x" , "      ", "#   # ", " # #  ", "  #   ", " # #  ", "#   #"},
    {"y" , "      ", "#    #", " #  # ", "  ##  ", "  #   ", " #    "},
    {"z" , "      ", "      ", "######", "   ## ", "  ##  ", "######"},
    {"0" , "      ", " #### ", "#    #", "#    #", "#    #", " #### "},
    {"1" , "      ", "  ##  ", " ###  ", "  ##  ", "  ##  ", "######"},
    {"2" , "      ", " #### ", "     #", "  ### ", " #    ", " #####"},
    {"3" , "      ", " #### ", "     #", "  ### ", "     #", " #### "},
    {"4" , "      ", "#    #", "#    #", "######", "     #", "     #"},
    {"5" , "      ", "######", "#     ", "######", "     #", "######"},
    {"6" , "      ", " #### ", "#     ", "##### ", "#    #", " #### "},
    {"7" , "      ", "######", "     #", "    # ", "   #  ", "  #   "},
    {"8" , "      ", " #### ", "#    #", " #### ", "#    #", " #### "},
    {"9" , "      ", " #### ", "#    #", " #####", "     #", " #### "},
    {" " , "      ", "      ", "      ", "      ", "      ", "      "},
    {"." , "      ", "      ", "      ", "      ", "      ", "  ##  "},
    {"+" , "      ", "      ", "  #   ", "##### ", "  #   ", "      "},
    {"-" , "      ", "      ", "      ", "######", "      ", "      "},
    {"!" , "      ", "  ##  ", "  ##  ", "  ##  ", "      ", "  ##  "}
};

void get_indices(const char * message, uint8_t *indices) {
    uint8_t index=0;
    uint8_t indices_index=0;
    char current_char;
    while (message[index] != '\0') {
        current_char = message[index]; 
        if (current_char >= 97) {
            index = 26;
        }
        else if (current_char >= 64) {
            index = 0;
        }
        else if (current_char >= 48) {
            index = 51;
        }
        else {
            index = 62;
        }
        while (current_char != ascii_art[index][0][0]) {
            index++;
        };
        indices[indices_index] = index;
        indices_index++;
    }
}

void display_banner(const char *message , uint8_t banner_len)
{   
    uint8_t * indices = (uint8_t *)malloc(25);
    get_indices(message, indices);
    for (int row = 0; row < 6; ++row)
    {
        for (int current_char_row = 0; current_char_row < banner_len; ++current_char_row)
        {
            uart_write_bytes(DISPLAY_BANNER_UART, ascii_art[indices[current_char_row]][row], 6);
        }
        uart_write_bytes(DISPLAY_BANNER_UART, "\n", 1);
    }
}

static void display_banner_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    int intr_alloc_flags = 0;

    uart_driver_install(GET_BANNER_UART, BUF_SIZE , BUF_SIZE, 0, NULL, intr_alloc_flags);
    uart_param_config(GET_BANNER_UART, &uart_config);
    uart_set_pin(GET_BANNER_UART, GET_BANNER_TX, GET_BANNER_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(DISPLAY_BANNER_UART, BUF_SIZE , BUF_SIZE, 0, NULL, intr_alloc_flags);
    uart_param_config(DISPLAY_BANNER_UART, &uart_config);
    uart_set_pin(DISPLAY_BANNER_UART, DISPLAY_BANNER_TX, DISPLAY_BANNER_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    while (1)
    {
        // Read data from the UART
        int len = uart_read_bytes(GET_BANNER_UART, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);

        if (len)
        {
            display_banner( (char *) data, len); // Imprimir la pancarta
        }

    }
}

void app_main(void)
{
    xTaskCreate(display_banner_task, "display_banner_task", ECHO_TASK_STACK_SIZE * 2, NULL, 10, NULL);
}