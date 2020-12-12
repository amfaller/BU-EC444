/*	EC444 Quest 1 Skill 9
*	Servos
*	September 11, 2020
* 	Author: Tony Faller	  */

/* Note: This code is a slightly modifed version of the provided mcpwm_servo_control_example.c given as an example */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define SERVO_MIN_PULSEWIDTH 550 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2550 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 180 //Maximum angle in degree upto which servo can rotate (-90 to +90)


/* This function calculates pulse width for per-degree rotation */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation){
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}


void app_main(void){
	uint32_t angle, count;

	/* Initialize MCPWN GPIO */
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);    // Set GPIO 18 as PWM0A, to which servo is connected

	/* Configure MCPWN */
	mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings

    /* Sweep servo through full range, from left (-90 degrees) to right (+90 degrees) */
    while (1) {
        for (count = 0; count < SERVO_MAX_DEGREE; count++) {
            printf("Angle of rotation: %d\n", count-90);
            angle = servo_per_degree_init(count);
            printf("pulse width: %dus\n", angle);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
            vTaskDelay(10);     //Add delay because servo takes time to rotate... If no delay, servo stutters
        }
    }


}