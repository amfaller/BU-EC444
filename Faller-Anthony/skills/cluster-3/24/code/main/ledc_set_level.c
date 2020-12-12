/* EC444 Quest01 Skill24
*  Using PWM to Control Power to an LED - Keyboard Set
*  October 18, 2020
*  Author: Tony Faller */

/* Note: This code is modified from https://github.com/espressif/esp-idf/tree/0289d1c/examples/peripherals/ledc */


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"

/*
 * About this example
 *
 * 1. Start with initializing LEDC module:
 *    a. Set the timer of LEDC first, this determines the frequency
 *       and resolution of PWM.
 *    b. Then set the LEDC channel you want to use,
 *       and bind with one of the timers.
 *
 * 2. You need first to install a default fade function,
 *    then you can use fade APIs.
 *
 * 3. You can also set a target duty directly without fading.
 *
 * 4. This example uses GPIO18/19/4/5 as LEDC output,
 *    and it will change the duty repeatedly.
 *
 * 5. GPIO18/19 are from high speed channel group.
 *    GPIO4/5 are from low speed channel group.
 *
 */
#ifdef CONFIG_IDF_TARGET_ESP32
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)                         // Use this pin for this skill
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (19)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1
#endif
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO       (4)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (5)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM       (4)                      // Number of LEDs to Control
#define LEDC_TEST_DUTY_MAX     (4000)                   // Max duty cycle
#define LEDC_TEST_FADE_TIME    (3000)                   // Time allocated to change intensity

#define LEDC_TEST_DUTY_0       0 
#define LEDC_TEST_DUTY_1       1 * (LEDC_TEST_DUTY_MAX / 10)  
#define LEDC_TEST_DUTY_2       2 * (LEDC_TEST_DUTY_MAX / 10)
#define LEDC_TEST_DUTY_3       3 * (LEDC_TEST_DUTY_MAX / 10)
#define LEDC_TEST_DUTY_4       4 * (LEDC_TEST_DUTY_MAX / 10) 
#define LEDC_TEST_DUTY_5       5 * (LEDC_TEST_DUTY_MAX / 10) 
#define LEDC_TEST_DUTY_6       6 * (LEDC_TEST_DUTY_MAX / 10)  
#define LEDC_TEST_DUTY_7       7 * (LEDC_TEST_DUTY_MAX / 10)
#define LEDC_TEST_DUTY_8       8 * (LEDC_TEST_DUTY_MAX / 10)
#define LEDC_TEST_DUTY_9       9 * (LEDC_TEST_DUTY_MAX / 10) 

void app_main(void)
{
    int ch;

    /* Prepare and set configuration of timers that will be used by LED Controller */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);

    /* Prepare and set configuration of timer1 for low speed channels */
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);


    /* Prepare individual configuration for each channel of LED Controller by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer, then frequency and bit_num of these channels will be the same */
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {
            .channel    = LEDC_HS_CH0_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER
        },
        {
            .channel    = LEDC_HS_CH1_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH1_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER
        },
        {
            .channel    = LEDC_LS_CH2_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH2_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_LS_TIMER
        },
        {
            .channel    = LEDC_LS_CH3_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_LS_CH3_GPIO,
            .speed_mode = LEDC_LS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_LS_TIMER
        },
    };

    /* Set LED Controller with previously prepared configuration */
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    /* Initialize fade service */
    ledc_fade_func_install(0);

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0,
      256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    /* Input Buffer */
    int level = 0;   

    while (1) {

        /* Read input buffer */
        printf("Enter LED brightness level (0-9): ");
        scanf("%d", &level);

        /* Clear input buffer */
        int c;
        while ( (c = getchar()) != '\n' && c != EOF ) { }

        /* Set level according to buffer */
        switch(level){
            case 0:
                printf("0\n");
                level = LEDC_TEST_DUTY_0;
                break;
            case 1:
                printf("1\n");
                level = LEDC_TEST_DUTY_1;
                break;
            case 2:
                printf("2\n");
                level = LEDC_TEST_DUTY_2;
                break;
            case 3:
                printf("3\n");
                level = LEDC_TEST_DUTY_3;
                break;
            case 4:
                printf("4\n");
                level = LEDC_TEST_DUTY_4;
                break;
            case 5:
                printf("5\n");
                level = LEDC_TEST_DUTY_5;
                break;
            case 6:
                printf("6\n");
                level = LEDC_TEST_DUTY_6;
                break;
            case 7:
                printf("7\n");
                level = LEDC_TEST_DUTY_7;
                break;
            case 8:
                printf("8\n");
                level = LEDC_TEST_DUTY_8;
                break;
            case 9:
                printf("9\n");
                level = LEDC_TEST_DUTY_9;
                break;
            default:
                printf("\nInvalid level, using input as raw duty value\n");
                break;
        }

        /* Make sure raw level doesn't break the LED */
        if(level > 4000) {level = 4000;}
        if(level < 0)    {level = 0;}

        /* Change LED Brightness */
        printf("LEDC fade to duty = %d\n\n", level);
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, level, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                    ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);


        // printf("3. LEDC set duty = %d without fade\n", level);
        // for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        //     ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, level);
        //     ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        // }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
