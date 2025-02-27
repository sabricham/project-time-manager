//--------------------------------------------------------------------------------------
/* 
*   Includes 
*/
//--------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

//--------------------------------------------------------------------------------------
/* 
*   Components 
*/
//--------------------------------------------------------------------------------------
#include "display.h"
#include "manager.h"
#include "encoder.h"
#include "led.h"
#include "network.h"

#include "queueHandler.h"
#include "taskHandler.h"

//--------------------------------------------------------------------------------------
/* 
*   Public variables & defines 
*/
//--------------------------------------------------------------------------------------

#define TAG "Main"
QueueHandle_t mainQueue = NULL;
queueMessage mainQueueMessage;
int mainMessagesParams[MESSAGE_PARAMS_LENGTH];

//--------------------------------------------------------------------------------------
/* 
*   Pulic functions & routines 
*/
//--------------------------------------------------------------------------------------

/*
*   Starting point of the code
*/
void app_main()
{
    ESP_LOGW(TAG, "System startup procedure");
    ESP_LOGW(TAG, "-------------------------------------------");
    ESP_LOGW(TAG, "           Time Manager Device             ");
    ESP_LOGW(TAG, "                 v0.01                     ");
    ESP_LOGW(TAG, "               Prototype                   ");
    ESP_LOGW(TAG, "-------------------------------------------");

    ESP_LOGW(TAG, "Starting system tasks");
    StartTask(EncoderTask, "Encoder", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 2);
    StartTask(ManagerTask, "Manager", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 3);
    StartTask(DisplayTask, "Display", configMINIMAL_STACK_SIZE * 4, tskIDLE_PRIORITY + 4);
    //StartTask(NetworkTask, "Network", configMINIMAL_STACK_SIZE * 4, tskIDLE_PRIORITY + 3);
    StartTask(LedTask, "Led", configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 2);
    ESP_LOGW(TAG, "System tasks started successfully");

    
    //fan_thread_entry(NULL);
    
    ESP_LOGW(TAG, "Starting task");
    CreateQueue(&mainQueue, &mainQueueMessage, 10, TAG);
    ESP_LOGW(TAG, "Task started correctly");

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(mainQueue, &mainQueueMessage, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }

        // Task logic
        vTaskDelay(1);
    }

    ESP_LOGE(TAG, "This section should not be reached");
    return;
}
