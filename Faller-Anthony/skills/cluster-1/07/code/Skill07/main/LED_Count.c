/* EC444 Quest01 Skill07
*  Using GPIO to Drive LEDs
*  September 9, 2020
*  Author: Tony Faller */

/* Note: This code is a modified version of the Blink sample project */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Define output pins for LEDs */
#define LED3_GPIO 27
#define LED2_GPIO 33
#define LED1_GPIO 15
#define LED0_GPIO 32

int LED3_state=0, LED2_state=0, LED1_state=0, LED0_state=0;   // LEDs initially off
int num=0;  // Integer version of counter

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(LED3_GPIO);
    gpio_pad_select_gpio(LED2_GPIO);
    gpio_pad_select_gpio(LED1_GPIO);
    gpio_pad_select_gpio(LED0_GPIO);

    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED3_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED0_GPIO, GPIO_MODE_OUTPUT);

    /* Counting loop */
    while(1) {
      printf("%d\n", num);
      gpio_set_level(LED3_GPIO, LED3_state);
      gpio_set_level(LED2_GPIO, LED2_state);
      gpio_set_level(LED1_GPIO, LED1_state);
      gpio_set_level(LED0_GPIO, LED0_state);

      vTaskDelay(1000 / portTICK_PERIOD_MS);

      /* One way to count in binary (up to 15), IMO easier than converting dec->bin */
      if(num == 7 || num == 15)
        LED3_state = ~LED3_state;
      if(num == 3 || num == 7 || num == 11 || num == 15)
        LED2_state = ~LED2_state;
      LED1_state = LED1_state ^ LED0_state;
      LED0_state = ~LED0_state;

      /* Wraparound */
      if(num == 15)
        num = 0;
      else
        num += 1;
    }
}
