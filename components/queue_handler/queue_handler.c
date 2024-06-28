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