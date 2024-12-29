#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

/* Components */
#include "button.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Button"
QueueHandle_t button_queue = NULL;
queue_message button_queue_message;

/* Public variables & defines */

/* Private functions & routines */

/* Public functions & routines */

void button_task()
{
    ESP_LOGI(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    start_queue(&button_queue, &button_queue_message, 10, TAG);

    ESP_LOGI(TAG, "Task started correctly");

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(button_queue, &button_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }
        
        esp_task_wdt_reset();
        vTaskDelay(1);
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}