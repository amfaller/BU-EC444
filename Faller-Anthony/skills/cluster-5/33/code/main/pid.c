/* EC444 Quest05 Skill32
*  Car Velocity via Optical Encoder
*  Novermber 20, 2020
*  Author: Tony Faller  */

/* Sourced from https://github.com/petemadsen/esp32/blob/master/hc-sr04/main/hcsr04.c */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#include "driver/gpio.h"

#include "sdkconfig.h"

/* Stuff for PID LED control */
#define GPIO_RED_LED		27
#define GPIO_GREEN_LED		33
#define GPIO_BLUE_LED		15

uint32_t level_red_LED = 0;
uint32_t level_green_LED = 0;
uint32_t level_blue_LED = 0;

#define SETPOINT_DISTANCE 30	// 30cm setpoint


/* Stuff for ultrasonic sensor */
static const char* NEC_TAG = "HCSR04";

#define RMT_TX_CHANNEL    RMT_CHANNEL_1     /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM  17     /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL    RMT_CHANNEL_0     /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM  16     /*!< GPIO number for receiver */

#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

#define HCSR04_MAX_TIMEOUT_US  25000   /*!< RMT receiver timeout value(us) */


#define US2TICKS(us)    (us / 10 * RMT_TICK_10_US)
#define TICKS2US(ticks) (ticks * 10 / RMT_TICK_10_US)


////////////////////////////////////////////////////////////////////////////////
// Ultrasonic Sensor ////////////////////////////////////////////////////////////

static inline void set_item_edge(rmt_item32_t* item, int low_us, int high_us)
{
    item->level0 = 0;
    item->duration0 = US2TICKS(low_us);
    item->level1 = 1;
    item->duration1 = US2TICKS(high_us);
}


/*
 * @brief RMT transmitter initialization
 */
static void nec_tx_init()
{
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;

    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 0;    // off

    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    ESP_ERROR_CHECK(rmt_config(&rmt_tx));
    ESP_ERROR_CHECK(rmt_driver_install(rmt_tx.channel, 0, 0));
}


/*
 * @brief RMT receiver initialization
 */
static void nec_rx_init()
{
    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_RX_CHANNEL;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = false;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = US2TICKS(HCSR04_MAX_TIMEOUT_US);
    ESP_ERROR_CHECK(rmt_config(&rmt_rx));
    ESP_ERROR_CHECK(rmt_driver_install(rmt_rx.channel, 1000, 0));
}


/**
 * @brief RMT receiver demo, this task will print each received NEC data.
 *
 */
static void rx_task()
{
    printf(">> Beginning rx task\n");

    printf(" - Initializing rx...\n");
    int channel = RMT_RX_CHANNEL;

    nec_rx_init();
    printf(" - rx initialization complete.\n");

    uint32_t rx_size = 0;
    RingbufHandle_t rb = NULL;
    rmt_item32_t *item = NULL;

    printf(">> Beginning rx reception...\n");
    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(channel, &rb);
    rmt_rx_start(channel, 1);

    while (rb)
    {
        // printf(">> RB condition met\n");

        //try to receive data from ringbuffer.
        //RMT driver will push all the data it receives to its ringbuffer.
        //We just need to parse the value and return the spaces of ringbuffer.
        item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1000);
        // printf(" - Something has been received from ring buffer\n");

        if (item)
        {
            // printf(" - Item condition met\n");
            for (int i=0; i<rx_size / sizeof(rmt_item32_t); ++i){
                ESP_LOGI(NEC_TAG, "RMT RCV -- %d:%d | %d:%d : %.1fcm",
                        item[i].level0, TICKS2US(item[i].duration0),
                        item[i].level1, TICKS2US(item[i].duration1),
                        (float)TICKS2US(item[i].duration0) / 58.2);

	            //Change LED states depending on distance
	            if((float)TICKS2US(item[i].duration0) / 58.2 < SETPOINT_DISTANCE){
					level_red_LED = 1;
					level_green_LED = 0;
					level_blue_LED = 0;
	            }
	            else if((float)TICKS2US(item[i].duration0) / 58.2 == SETPOINT_DISTANCE){
					level_red_LED = 0;
					level_green_LED = 1;
					level_blue_LED = 0;
	            }
	            else if((float)TICKS2US(item[i].duration0) / 58.2 > SETPOINT_DISTANCE){
					level_red_LED = 0;
					level_green_LED = 0;
					level_blue_LED = 1;
	            }
	            else{
					level_red_LED = 1;
					level_green_LED = 1;
					level_blue_LED = 1;
	            }
	        }

            //after parsing the data, return spaces to ringbuffer.
            vRingbufferReturnItem(rb, (void*) item);
        } else {
            printf(">> Item condition failed\n");
            vTaskDelay(10);
//            break;
        }
    }

    vTaskDelete(NULL);
}

static const rmt_item32_t morse_esp[] = {
    // E : dot
    {{{ US2TICKS(20), 0, US2TICKS(180), 1 }}}, // dot
    {{{ 16, 0, 144, 1 }}}, // dot
    // // S : dot, dot, dot
    // {{{ 32767, 0, 32767, 1 }}}, // dot
    // {{{ 32767, 0, 32767, 1 }}}, // dot
    // {{{ 32767, 0, 32767, 1 }}}, // dot
    // {{{ 32767, 0, 32767, 1 }}}, // SPACE
    // // P : dot, dash, dash, dot
    // {{{ 32767, 0, 32767, 1 }}}, // dot
    // {{{ 32767, 0, 32767, 1 }}},
    // {{{ 32767, 0, 32767, 1 }}}, // dash
    // {{{ 32767, 0, 32767, 1 }}},
    // {{{ 32767, 0, 32767, 1 }}}, // dash
    // {{{ 32767, 0, 32767, 1 }}}, // dot
    // RMT end marker
    {{{ 0, 1, 0, 0 }}}
};

/**
 * @brief RMT transmitter demo, this task will periodically send NEC data. (100 * 32 bits each time.)
 *
 */
static void tx_task()
{
    vTaskDelay(10);
    printf(">> Beginning tx task\n");

    printf(" - Initializing tx...\n");
    nec_tx_init();

    int channel = RMT_TX_CHANNEL;
    printf(" - tx initialization complete.\n");

    int item_num = 1;
    rmt_item32_t item[item_num];
    for (int i=0; i<item_num; ++i)
        set_item_edge(&item[i], 20, 180);
//  set_item_edge(&item[1], factor * 70, factor * 30);
    printf(" - Item creation complete.\n");
    printf(">> Sending tx data...\n");

    for (;;)
    {
        ESP_LOGI(NEC_TAG, "RMT TX DATA");

        // To send data according to the waveform items.
        rmt_write_items(channel, morse_esp, sizeof(morse_esp) / sizeof(morse_esp[0]), true);
        // Wait until sending is done.
        rmt_wait_tx_done(channel, portMAX_DELAY);
        // before we free the data, make sure sending is already done.

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}


void hcsr04_init()
{
    xTaskCreate(rx_task, "rx_task", 2048, NULL, 10, NULL);
    xTaskCreate(tx_task, "tx_task", 2048, NULL, 10, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// PID Stuff ///////////////////////////////////////////////////////////////////

/* Initialization function for LEDs */
static void init_LED(){
	// Select GPIO
	gpio_pad_select_gpio(GPIO_RED_LED);
	gpio_pad_select_gpio(GPIO_GREEN_LED);
	gpio_pad_select_gpio(GPIO_BLUE_LED);

	// Set as outputs
	gpio_set_direction(GPIO_RED_LED, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_GREEN_LED, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_BLUE_LED, GPIO_MODE_OUTPUT);

}

/* Task to change LEDs based on error */
static void LED_task(){
	while(1){
		gpio_set_level(GPIO_RED_LED, level_red_LED);
		gpio_set_level(GPIO_GREEN_LED, level_green_LED);
		gpio_set_level(GPIO_BLUE_LED, level_blue_LED);

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Main ////////////////////////////////////////////////////////////////////////

void app_main(void *ignore){
    printf(">> Begin\n");

    // Initialize LEDs
    init_LED();

    // Create LED task
    xTaskCreate(LED_task, "LED_task", 2048, NULL, 11, NULL);

    // Start the ultrasonic sensor
    hcsr04_init();
}