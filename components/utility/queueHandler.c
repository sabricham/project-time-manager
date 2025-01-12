#include "queueHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Queue Handler"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

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
*   Send a message over a queue
*/
void SendMessage(QueueHandle_t queue, uint8_t senderID, uint8_t deviceID, uint8_t messageID, int *params)
{
    queueMessage messageTX;  

    memset(&messageTX, 0, sizeof(queueMessage)); 
    messageTX.senderID = senderID;
    messageTX.deviceID = deviceID;
    messageTX.messageID = messageID;
    if(params != NULL)
        memcpy(messageTX.params, params, sizeof(int) * MESSAGE_PARAMS_LENGTH);

    while(queue == NULL)
        vTaskDelay(100);
    xQueueSend(queue, &messageTX, MESSAGE_PARAMS_SEND_TICKS);
}

/*
*   Send a message over a queue
*/
void CreateQueue(QueueHandle_t * queue, queueMessage * message, uint8_t size, char * taskTAG)
{
    memset(message, 0, sizeof(queueMessage));
    *queue = xQueueCreate(size, sizeof(queueMessage));
    if (*queue == NULL) 
    {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }
    ESP_LOGI(TAG, "Queue for %s created correctly", taskTAG);
}