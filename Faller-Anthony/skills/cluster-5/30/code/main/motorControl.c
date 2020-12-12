/* brushed dc motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

/*	Tony's Notes, Nov 18 2020
 *	1. It appears that MCPWN_OPR_A controls the left wheel and MCPWN_OPR_B controls the right wheel. Setting either to low will disable that wheel.
 *	2. The GPIO output states for GPIO_IN_3 and GPIO_IN_4 are the opposite of what I would have expected, but it's because clockwise on one wheel is counterclockwise on the other.
*/

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define GPIO_PWM0A_OUT 26   //Set GPIO 26 (A0) as PWM0A (EN1-2)
#define GPIO_PWM0B_OUT 14   //Set GPIO 14 as PWM0B (EN3-4)

#define GPIO_IN1	25		// Set GPIO 25 (A1) as IN1
#define GPIO_IN2	18		// Set GPIO 18 (MO) as IN2
#define GPIO_IN3	12		// Set GPIO 12 as IN3
#define GPIO_IN4	32		// Set GPIO 32 as IN4

static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);

    gpio_pad_select_gpio(GPIO_IN1);
    gpio_pad_select_gpio(GPIO_IN2);
    gpio_pad_select_gpio(GPIO_IN3);
    gpio_pad_select_gpio(GPIO_IN4);

    /* Set the GPIO as a push/pull output */
    gpio_set_direction(GPIO_IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_IN3, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_IN4, GPIO_MODE_OUTPUT);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
	gpio_set_level(GPIO_IN1, 0);
    gpio_set_level(GPIO_IN2, 1);
    gpio_set_level(GPIO_IN3, 1);
    gpio_set_level(GPIO_IN4, 0);

    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state

}

/**
 * @brief left motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward_right(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
	gpio_set_level(GPIO_IN1, 0);
    gpio_set_level(GPIO_IN2, 1);
    gpio_set_level(GPIO_IN3, 1);
    gpio_set_level(GPIO_IN4, 0);

    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    // mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    // mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state

}

/**
 * @brief right motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward_left(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
	gpio_set_level(GPIO_IN1, 0);
    gpio_set_level(GPIO_IN2, 1);
    gpio_set_level(GPIO_IN3, 1);
    gpio_set_level(GPIO_IN4, 0);

    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    // mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    // mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state

}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
	gpio_set_level(GPIO_IN1, 1);
    gpio_set_level(GPIO_IN2, 0);
    gpio_set_level(GPIO_IN3, 0);
    gpio_set_level(GPIO_IN4, 1);



    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state


    // mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{

	gpio_set_level(GPIO_IN1, 0);
    gpio_set_level(GPIO_IN2, 0);
    gpio_set_level(GPIO_IN3, 0);
    gpio_set_level(GPIO_IN4, 0);

    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    while (1) {
    	printf("Forward...\n");
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 65.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf("Backward...\n");
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf("Right...\n");
        brushed_motor_forward_right(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(1000 / portTICK_RATE_MS);

        printf("Left...\n");
        brushed_motor_forward_left(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(1000 / portTICK_RATE_MS);

        printf("Backward...\n");
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf("Forward faster...\n");
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 85.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf("Stop...\n\n");
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    printf("Testing brushed motor...\n");
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_examlpe_brushed_motor_control", 4096, NULL, 5, NULL);
}
