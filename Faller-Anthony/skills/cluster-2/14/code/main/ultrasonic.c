/* EC444 Quest02 Skill14
*  Ultrasonic Range Sensor (ADC)
*  September 24, 2020
*  Author: Tony Faller  */

/* Note" This code is a slightly modified version of Skill12, which was taken from https://github.com/espressif/esp-idf/tree/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc */

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

/* Define and Initialize stuff for ADC */
#define DEFAULT_VREF    1100        // Default ADC reference voltage in mV

static esp_adc_cal_characteristics_t *adc_chars;        // Pointer to empty structure used to store ADC characteristics
static const adc_channel_t channel = ADC_CHANNEL_6;     // GPIO34 because we're using ADC1
static const adc_atten_t atten = ADC_ATTEN_DB_0;       // Attenuation to characterize
static const adc_unit_t unit = ADC_UNIT_1;              // ADC to characterize (ADC_UNIT_1 or ADC_UNIT_2)


////////////////////////////////////////////////////////////////////////////////
// Main ////////////////////////////////////////////////////////////////////////
void app_main(void){

    /* Configure for ADC1 */
    adc1_config_width(ADC_WIDTH_BIT_10);
    adc1_config_channel_atten(channel, atten);

    /* Characterize ADC */
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_10, DEFAULT_VREF, adc_chars);

    /* Continuously sample ADC1 */
    while(1){
        uint32_t adc_reading = 0;

        // Sample 
        adc_reading += adc1_get_raw((adc1_channel_t)channel);

        // Convert adc_reading to voltage
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        // Convert voltage to distance in mm. Per the data sheet, a 10-bit ADC reading can be multiplied by 5 to get the distance
        uint32_t d_mm = adc_reading * 5 + 300;		// The +300 is a calibration thing, determined after collecting some data points

        // Print to console
        printf("Raw: %d\tVoltage: %dmV\t Distance: %dmm\n", adc_reading, voltage, d_mm);

        // Wait 2 seconds before next sample
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

}