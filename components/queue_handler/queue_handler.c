#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Components */
#include "queue_handler.h"

/* Private variables & defines */
#define TAG "Queue Handler"

/* Private functions & routines */

/* Public functions & routines */
void send_message(QueueHandle_t queue, uint8_t sender_id, uint8_t device_id, uint8_t message_id, int *params)
{
    queue_message tx_mess;  

    memset(&tx_mess, 0, sizeof(queue_message)); 
    tx_mess.sender_id = sender_id;
    tx_mess.device_id = device_id;
    tx_mess.message_id = message_id;
    if(params != NULL)
        memcpy(tx_mess.params, params, sizeof(int) * MESSAGE_PARAMS_LENGTH);

    while(queue == NULL)
        vTaskDelay(100);
    xQueueSend(queue, &tx_mess, MESSAGE_PARAMS_SEND_TICKS);

    return;
}

void start_queue(QueueHandle_t * queue, queue_message * message, uint8_t size, char * task_tag)
{
    ESP_LOGI(TAG, "Creating queue for %s", task_tag);
    memset(message, 0, sizeof(queue_message));
    *queue = xQueueCreate(size, sizeof(queue_message));
    if (*queue == NULL) 
    {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }
    ESP_LOGI(TAG, "Queue for %s created correctly", task_tag);
    return;
}