/* EC444 Quest04 Skill28
*  Leader Election
*  November 7, 2020
*  Author: Tony Faller */

/* Note: This code is partially sourced from https://github.com/espressif/esp-idf/tree/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc */

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/param.h>

// RTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

// Wifi Communication
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

// UDP
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/* Define macros for ADC */
#define DEFAULT_VREF    1100        // Default ADC reference voltage in mV

/* Define macros for timer */
#define TIMER_DIVIDER         16                                    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)      // to seconds
#define TIMER_INTERVAL_SEC    (0.1)                                 // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1                                     // Testing will be done with auto reload
#define TIMER_HOURS           0                                     // Timer hours unit, change to change feeding interval
#define TIMER_MINUTES         1                                     // Timer minutes unit, change to change feeding interval

/* Define ID/color */
#define ID 2
#define COLOR 'R'

/* Define timeout values */
#define TIMEOUT_ELECTION      10

/* Handlers for each task */
TaskHandle_t udpServerHandle;   // UDP receive task
TaskHandle_t udpClientHandle;   // UDP send task

// Variables for my ID, minVal and status plus string fragments
char start = 0x1B;
char myID = (char) ID;
char leaderID = (char) ID;
int isLeader = 0;
int len_out = 4;

// Flag to check if heartbeat is received
int recHeartbeat = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDP Communication Stuff /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client */
/* https://github.com/espressif/esp-idf/blob/master/examples/protocols/sockets/udp_server/main/udp_server.c */

// #ifdef CONFIG_EXAMPLE_IPV4
#define HOST_IP_ADDR "192.168.86.29"        // FOB1
#define HOST_IP_ADDR_2 "192.168.86.25"      // FOB3
// #else
// #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
// #endif

#define PORT 64209

/* Tags for log messages */
static const char *TAGTX = "electionTx";
static const char *TAGRX = "electionRx";


/* Task to send payload to server (UDP Client) */
static void udp_client_task(void *pvParameters)
{
    // First FOB
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    // Second FOB
    char addr_str_2[128];
    int addr_family_2;
    int ip_protocol_2;

    while (1) {

// #ifdef CONFIG_EXAMPLE_IPV4
        // First FOB
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        // Second FOB
        struct sockaddr_in dest_addr_2;
        dest_addr_2.sin_addr.s_addr = inet_addr(HOST_IP_ADDR_2);
        dest_addr_2.sin_family = AF_INET;
        dest_addr_2.sin_port = htons(PORT);
        addr_family_2 = AF_INET;
        ip_protocol_2 = IPPROTO_IP;
        inet_ntoa_r(dest_addr_2.sin_addr, addr_str_2, sizeof(addr_str_2) - 1);
// #else // IPV6
//         struct sockaddr_in6 dest_addr = { 0 };
//         inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
//         dest_addr.sin6_family = AF_INET6;
//         dest_addr.sin6_port = htons(PORT);
//         // Setting scope_id to the connecting interface for correct routing if IPv6 Local Link supplied
//         dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
//         addr_family = AF_INET6;
//         ip_protocol = IPPROTO_IPV6;
//         inet6_ntoa_r(dest_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
// #endif

        // First FOB
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAGTX, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAGTX, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        // Second FOB
        int sock_2 = socket(addr_family_2, SOCK_DGRAM, ip_protocol_2);
        if (sock < 0) {
            ESP_LOGE(TAGTX, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAGTX, "Socket created, sending to %s:%d", HOST_IP_ADDR_2, PORT);

        while (1) {

            // Reset payload
            char payload[200];

            // Set payload
            sprintf(payload, "%x%x%x%x", start, myID, leaderID, (char)isLeader);
            //printf("leaderID = %c\n", payload[2]);    // For debugging

            // Send payload to First FOB
            int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAGTX, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAGTX, "Message sent: %s", payload);

            // Send payload to Second FOB
            int err_2 = sendto(sock_2, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr_2, sizeof(dest_addr_2));
            if (err_2 < 0) {
                ESP_LOGE(TAGTX, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAGTX, "Message sent: %s", payload);

            // struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            // socklen_t socklen = sizeof(source_addr);
            // int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // // Error occurred during receiving
            // if (len < 0) {
            //     ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            //     break;
            // }
            // // Data received
            // else {
            //     rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string

            //     ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            //     ESP_LOGI(TAG, "%s", rx_buffer);
            // }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAGTX, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


/* Task to send payload to server (UDP Server) */
static void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1) {

        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        } else if (addr_family == AF_INET6) {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAGRX, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAGRX, "Socket created");

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAGRX, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAGRX, "Socket bound, port %d", PORT);

        while (1) {

            ESP_LOGI(TAGRX, "Waiting for data");
            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAGTX, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (source_addr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAGRX, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAGRX, "%s", rx_buffer);

                // Set heartbeat flag
                if(rx_buffer[3] == leaderID) {recHeartbeat = 1;}

                // printf("Received ID: %c\tMyID: %c\tLeaderID: %c\n", rx_buffer[2], myID+'0', leaderID);  // For debugging
                // if(myID == leaderID){printf("I am the leader!\n");}     // For debugging
                // printf("rx_buffer by index: %c %c %c %c %c\n", rx_buffer[0], rx_buffer[1] , rx_buffer[2] , rx_buffer[3] , rx_buffer[4]);

                // Set new leader if leader already chosen or received ID < Fob ID
                if(strlen(rx_buffer) == 5 && rx_buffer[4] == '1') {leaderID = rx_buffer[3];}
                else if(rx_buffer[3] < myID+'0') {leaderID = rx_buffer[3];}

                // // Reset payload
                // char payload[200];

                // // Set payload
                // sprintf(payload, "%x%x%x%x", start, myID, leaderID, (char)isLeader);

                // int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                // if (err < 0) {
                //     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                //     break;
                // }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAGRX, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization Stuff ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Function to initialize Wifi */
void init_wifi(){
    // Initialize Wifi
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
}

/* Function to end the election */
void election(){
    printf(">> Running election...\n");

    // Wait for election to timeout
    for(int i=0; i<TIMEOUT_ELECTION;i++){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Pause election tasks
    vTaskSuspend(udpServerHandle);
    vTaskSuspend(udpClientHandle);
    printf(" - Election tasks paused\n");

    // Change leaderID
    if(myID == leaderID)
        {isLeader = 1;}
    else
        {isLeader = 0;}

    printf(" - Election complete. LeaderID:");
    if(leaderID < 48) {printf("%c", leaderID+'0');}
    else if(leaderID > 57) {printf("%c", leaderID-'0');}
    else {printf("%c", leaderID);}
    printf("\n\n");
}

/* Function to restart the election */
void election_restart(){
    printf(">> Restarting election...\n");

    // Reset payload values
    myID = (char) ID;
    leaderID = (char) ID;
    isLeader = 0;

    // Resume election tasks
    vTaskResume(udpServerHandle);
    vTaskResume(udpClientHandle);

    // Call election
    election();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_main(void){

    // Initialize wifi
    init_wifi();

    // Create UDP client / server tasks
    xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET, 6, &udpServerHandle);
    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, &udpClientHandle);

    // Initial election
    election();

    // If you are the leader, send out your heartbeat. Else open your ears to hear heartbeat
    if(isLeader) {vTaskResume(udpClientHandle);}
    else {vTaskResume(udpServerHandle);}

    while(1){

        // If you're not the leader, check for heartbeat
        if(!isLeader){
            if(recHeartbeat){
                printf(">> Heartbeat received\n");
                recHeartbeat = 0;   // Reset heartbeat flag
            }
            else{
                printf(">> No heartbeat received in time\n");
                election_restart();
                if(isLeader) {vTaskResume(udpClientHandle);}
                else {vTaskResume(udpServerHandle);}
            }
        }

        else{
            printf("I am the leader, hear my heartbeat.\n");
        }
        

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

}