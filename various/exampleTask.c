#include "exampleTask.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Example Task"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

QueueHandle_t sampleQueue = NULL;
queueMessage sampleQueueMessage;
int exampleTaskMessagesParams[MESSAGE_PARAMS_LENGTH];

//======================================================================================
/* 
*   Private functions & routines
*/
//======================================================================================

//======================================================================================
/* 
*   Public variables & defines
*/
//======================================================================================

//======================================================================================
/* 
*   Public functions & routines
*/
//======================================================================================

/*
*   This is an example of a task
*/
void ExampleTask()
{
    ESP_LOGI(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    start_queue(&sampleQueue, &sampleQueueMessage, 10, TAG);

    ESP_LOGI(TAG, "Task started correctly");

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(sampleQueue, &sampleQueueMessage, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }
        
        esp_task_wdt_reset();
        vTaskDelay(1);
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}