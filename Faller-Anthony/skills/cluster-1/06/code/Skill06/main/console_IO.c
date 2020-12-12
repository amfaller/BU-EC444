/* EC444 Quest01 Skill06
*  Console I/O
*  September 8, 2020
*  Author: Tony Faller */

/* Note: This code is a modified combination of the Blink sample project
*  and the example code on the Console I/O brief on the course website. */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink*/
#define BLINK_GPIO 13

void app_main(void)
{
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
      256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);

    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int LEDstate = 0;       // LED is initally off
    int modeSelect = 0;     // Initial mode is LED toggle
    int oldMode = -1;       // Mode comparator
    int dec = 0;            // Number to be converted to hex
    char buf[80];           // Input buffer, maximum of 80 characters 

    while(1) {

        /* Mode Selection*/
        if(buf[0] == 's'){
            modeSelect += 1;
            printf("Mode changed. ");
            strcpy(buf, ""); 
        }

        /* Toggle Mode */
        if(modeSelect == 0) {
            if(modeSelect != oldMode){
                printf("You are in toggle mode. Enter t to toggle the LED.\n");
                oldMode = modeSelect;
            }
            gets(buf);
            if(buf[0] == 't') {                         // Toggle LED on T input
                printf("Toggling the LED\n");
                LEDstate = ~LEDstate;
                gpio_set_level(BLINK_GPIO, LEDstate);
                strcpy(buf, "");                        // Clear input buffer
            }
        }

        /* Echo Mode */
        else if(modeSelect == 1){ 
            if(modeSelect != oldMode){
                printf("You are in echo mode.\n");
                oldMode = modeSelect;
            }
            gets(buf);
            if(buf[0] != '\0'){
                printf("echo: %s\n", buf);
                if(buf[0] == 's'){
                    modeSelect += 1;
                    printf("Mode changed. ");
                }
                strcpy(buf, "");
            }
        }

        /* Dec->Hex Mode */
        else if(modeSelect == 2){
            if(modeSelect != oldMode){
                printf("You are in dec->hex mode.\n");
                oldMode = modeSelect;
            }
            gets(buf);
            dec = atoi(buf);
            if(buf[0] == 's'){
                modeSelect = 0;
                printf("Mode changed. ");
                strcpy(buf, ""); 
            }
            else if (dec == 0 && buf[0] != '0') printf("Invalid input\n");
            else printf("Integer: %d\nHex: %x\n", dec, dec);
        } 

        /* modeSelect failsafe */
        else{
            modeSelect = 0;
            strcpy(buf, "");
        }        
    }
}