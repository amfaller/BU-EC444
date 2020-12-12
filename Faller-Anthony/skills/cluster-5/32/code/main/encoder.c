/* EC444 Quest05 Skill32
*  Car Velocity via Optical Encoder
*  Novermber 20, 2020
*  Author: Tony Faller  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"

// Motor control stuff
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

/* Define and Initialize stuff for ADC */
#define DEFAULT_VREF    1100        // Default ADC reference voltage in mV

static esp_adc_cal_characteristics_t *adc_chars;        // Pointer to empty structure used to store ADC characteristics
static const adc_channel_t channel = ADC_CHANNEL_3;     // GPI39 (A3) to use ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_11;       // Attenuation to characterize, changed from 0 to 11 to allow for a higher range
static const adc_unit_t unit = ADC_UNIT_1;              // ADC to characterize (ADC_UNIT_1 or ADC_UNIT_2)

/* Define stuff for motor control */
#define GPIO_PWM0A_OUT 26   //Set GPIO 26 (A0) as PWM0A (EN1-2)
#define GPIO_PWM0B_OUT 14   //Set GPIO 14 as PWM0B (EN3-4)

#define GPIO_IN1	25		// Set GPIO 25 (A1) as IN1
#define GPIO_IN2	18		// Set GPIO 18 (MO) as IN2
#define GPIO_IN3	12		// Set GPIO 12 as IN3
#define GPIO_IN4	32		// Set GPIO 32 as IN4

/* Handler for encoder task */
TaskHandle_t encoder_read_handle;

/* Global variable to indicate direction --> 1 means forward, 0 means backwards */
uint32_t direction = 1;

////////////////////////////////////////////////////////////////////////////////
// Optical Encoder Stuff ///////////////////////////////////////////////////////

/* Function to initialize ADC for optical encoder */
void encoder_init(){
	    /* Configure for ADC1 */
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, atten);

    /* Characterize ADC */
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

}

// Set of states (black/white)
typedef enum {
	STATE_BLACK,
	STATE_WHITE
} state_e;

/* Task to read optical encoder & calculate speed*/
static void encoder_read_task(){

	// Initial states of the wheels
	state_e currState = STATE_BLACK;
	state_e oldState = currState;

	while(1){

		// Revolution counter
		uint32_t revolutionCount = 0;

		// Sample for 1 second
		for(int i=0; i<100; i++){
	        uint32_t adc_reading = 0;

	        adc_reading += adc1_get_raw((adc1_channel_t)channel);

	        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

	        // Print to console (for debugging)
	        // printf("Raw: %d\tVoltage: %d\tColor: ", adc_reading, voltage);
	       	// if(voltage < 240) printf("white\n");
	       	// else printf("black\n");

	       	// Change state based on voltage
	       	if(voltage < 240)
	       		currState = STATE_WHITE;
	       	else
	       		currState = STATE_BLACK;

	       	// Initialize oldState
	       	if(i==0) {oldState = currState;}

	       	// Increment rotation counter if state changed
	       	if(currState != oldState){
	       		revolutionCount++;
	       		oldState = currState;
	       	}

	        // Wait 10ms before next sample
	        vTaskDelay(10 / portTICK_PERIOD_MS);
	    }

	    // Calculate speed in m/s based on revolution counter
	    float speed = (float)revolutionCount * 2.0 * 6.0 * 3.14 / 100.0;

	    // Calibration
	    if(speed < 0.4) {speed = 0;}

	    // Print speed to console
	    // printf("revolutionCount: %d\tCurrent speed: %.2f m/s\n", revolutionCount, speed);

	    // Print velocity to console
	    printf(" - Current velocity: ");
	    if(direction) {printf("+");}
	    else {printf("-");}
	    printf("%.2f m/s\n", speed);

    }
}

////////////////////////////////////////////////////////////////////////////////
// Motor control stuff /////////////////////////////////////////////////////////

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

    // Set direction
    direction = 1;

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

    // Set direction
    direction = 1;

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

    // Set direction
    direction = 1;

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

    // Set direction
    direction = 0;

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

    // Set direction
    direction = 1;

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
    	// Resume speed readings
    	// vTaskResume(encoder_read_handle);

    	printf(">> Forward...\n");
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 65.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf(">> Backward...\n");
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf(">> Right...\n");
        brushed_motor_forward_right(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(1000 / portTICK_RATE_MS);

        printf(">> Left...\n");
        brushed_motor_forward_left(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(1000 / portTICK_RATE_MS);

        printf(">> Backward...\n");
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 75.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf(">> Forward faster...\n");
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 85.0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        printf(">> Stop...\n\n");
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        vTaskDelay(2000 / portTICK_RATE_MS);

        //Suspend speed readings
        // vTaskSuspend(encoder_read_handle);

    }
}

////////////////////////////////////////////////////////////////////////////////
// Main ////////////////////////////////////////////////////////////////////////
void app_main(void){

	// Initialize encoder and pcnt
	encoder_init();

    // Continuously sample ADC1
    xTaskCreate(encoder_read_task, "encoder_read", 4096, NULL, 10, &encoder_read_handle);

    // Run motors
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_examlpe_brushed_motor_control", 4096, NULL, 9, NULL);
}