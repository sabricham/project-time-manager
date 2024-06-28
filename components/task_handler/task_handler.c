#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Components */
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Task Handler"

/* Private functions & routines */
void start_task(TaskHandle_t (* task)(void *), char * task_name, uint16_t stack_size, uint8_t priority)
{    
    ESP_LOGI(TAG, "Creating '%s' task", task_name);
    if(xTaskCreate((*task), task_name, stack_size, NULL, priority, NULL) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task");
        return;
    }
    ESP_LOGI(TAG, "Task '%s' created correctly", task_name);
    return;
}