#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

/* Components */
#include "manager.h"
#include "display.h"
#include "encoder.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Manager"
extern QueueHandle_t display_queue;
extern QueueHandle_t encoder_queue;
QueueHandle_t manager_queue = NULL;
queue_message manager_queue_message;

enum manager_internal_state{
    manager_state_startup,
    manager_state_idle,
    manager_state_timer,
    manager_state_stopwatch,
    manager_state_pomodoro,
    manager_state_settings
};
uint8_t manager_state = manager_state_idle;

TickType_t timer_seconds_counter = 0;

/* Private functions & routines */
uint8_t manager_startup()
{
    queue_message tx_mess;   

    // set encoder single mode
    memset(&tx_mess, 0, sizeof(queue_message)); 
    tx_mess.sender_id = SENDER_ID_MANAGER;
    tx_mess.device_id = DEVICE_ID_ENCODER;
    tx_mess.message_id = MESSAGE_ID_ENCODER_MODE_SINGLE;
    while(encoder_queue == NULL)
        vTaskDelay(100);
    xQueueSend(encoder_queue, &tx_mess, 10);

    // set new image to display
    memset(&tx_mess, 0, sizeof(queue_message)); 
    tx_mess.sender_id = SENDER_ID_MANAGER;
    tx_mess.device_id = DEVICE_ID_DISPLAY;
    tx_mess.message_id = MESSAGE_ID_DISPLAY_PAGE_IDLE;
    tx_mess.data[0] = 0;    //offset
    while(display_queue == NULL)
        vTaskDelay(100);
    xQueueSend(display_queue, &tx_mess, 10);

    return manager_state_idle;
}

uint8_t manager_idle()
{    
    queue_message tx_mess; 
    queue_message rx_mess; 
    int image_add_offset = 0;

    if(xQueueReceive(manager_queue, &rx_mess, 0))
    {
        if(rx_mess.sender_id == SENDER_ID_ENCODER)
        {
            switch(rx_mess.message_id)
            {
                case MESSAGE_ID_ENCODER_ANGLE_VARIATION:
                {
                    timer_seconds_counter = xTaskGetTickCount() / configTICK_RATE_HZ;

                    image_add_offset=rx_mess.data[0];

                    // set new image to display
                    memset(&tx_mess, 0, sizeof(queue_message)); 
                    tx_mess.sender_id = SENDER_ID_MANAGER;
                    tx_mess.device_id = DEVICE_ID_DISPLAY;
                    tx_mess.message_id = MESSAGE_ID_DISPLAY_PAGE_DIGITS;
                    //tx_mess.data[0] = image_add_offset;    //offset on previous image
                    tx_mess.data[0] = (timer_seconds_counter/60)*100 + (timer_seconds_counter%60);    //offset on previous image
                    ESP_LOGI(TAG, "counter: %ld, counter/60: %ld counterMod60: %ld", timer_seconds_counter, timer_seconds_counter/60, timer_seconds_counter%60);
                    while(display_queue == NULL)
                        vTaskDelay(100);
                    xQueueSend(display_queue, &tx_mess, 10);
                }
                break;
            }
        }
    } 

    return manager_state_idle;
}

uint8_t manager_timer()
{
    return manager_state_timer;
}

uint8_t manager_stopwatch()
{
    return manager_state_stopwatch;    
}

uint8_t manager_pomodoro()
{
    return manager_state_pomodoro;
}

uint8_t manager_settings()
{
    return manager_state_settings;
}

/* Public functions & routines */
void manager_task()
{
    ESP_LOGW(TAG, "Starting task");
    esp_task_wdt_add(NULL);
    start_queue(&manager_queue, &manager_queue_message, 10, TAG);
    manager_state = manager_state_startup;
    ESP_LOGW(TAG, "Task started correctly");

    while(1)
    {
        /*
        // Get messages from other tasks
        if(xQueueReceive(manager_queue, &manager_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }
        */
        
        switch(manager_state)
        {
            case manager_state_startup:
            {
                manager_state = manager_startup();
            }
            break;
            case manager_state_idle:
            {
                manager_state = manager_idle();
            }
            break;
            case manager_state_timer:
            {
                manager_state = manager_timer();
            }
            break;
            case manager_state_stopwatch:
            {
                manager_state = manager_stopwatch();
            }
            break;
            case manager_state_pomodoro:
            {
                manager_state = manager_pomodoro();
            }
            break;
            case manager_state_settings:
            {
                manager_state = manager_settings();
            }
            break;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}