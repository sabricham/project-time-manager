#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

/* Components */
#include "display.h"
#include "manager.h"
#include "encoder.h"
#include "led.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Public variables & defines */

#define TAG "Main"
QueueHandle_t main_queue = NULL;
queue_message main_queue_message;

/* Public functions & routines */

void app_main()
{
    ESP_LOGW(TAG, "System startup procedure");
    ESP_LOGW(TAG, "-------------------------------------------");
    ESP_LOGW(TAG, "           Time Manager Device             ");
    ESP_LOGW(TAG, "                 v0.01                     ");
    ESP_LOGW(TAG, "               Prototype                   ");
    ESP_LOGW(TAG, "-------------------------------------------");

    ESP_LOGW(TAG, "Starting system tasks");
    start_task(encoder_task, "Encoder", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 2);
    start_task(manager_task, "Manager", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 3);
    start_task(display_task, "Display", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 4);
    start_task(led_task, "Display", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 1);
    ESP_LOGW(TAG, "System tasks started successfully");
    
    ESP_LOGW(TAG, "Starting task");
    start_queue(&main_queue, &main_queue_message, 10, TAG);
    ESP_LOGW(TAG, "Task started correctly");

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(main_queue, &main_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }

        // Task logic
        vTaskDelay(1);
    }

    ESP_LOGE(TAG, "This section should not be reached");
    return;
}
