#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Components */
#include "sample.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Sample"
QueueHandle_t sample_queue = NULL;
queue_message sample_queue_message;

/* Public variables & defines */


/* Private functions & routines */

/* Public functions & routines */

void sample_task()
{
    ESP_LOGI(TAG, "Starting task");

    start_queue(&sample_queue, &sample_queue_message, 10, TAG);

    ESP_LOGI(TAG, "Task started correctly");

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(sample_queue, &sample_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }
        
        vTaskDelay(1);
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}