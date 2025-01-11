#include "encoder.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Encoder"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

QueueHandle_t encoderQueue = NULL;
extern QueueHandle_t managerQueue;
queueMessage encoderQueueMessage;
int encoderMessageParams[MESSAGE_PARAMS_LENGTH];

uint8_t encoderVariationMode = 0;

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
*   Entry point of the task
*/
void EncoderTask()
{    
    //======================================================================================
    ESP_LOGW(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    CreateQueue(&encoderQueue, &encoderQueueMessage, 10, TAG);
    encoderVariationMode = ENCODER_MODE_SINGLE;

    KY040_t ky040Encoder;
    KY040Init(&ky040Encoder, ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PIN_SWITCH, ENCODER_ANGLE_INCREMENT);

    ESP_LOGW(TAG, "Task started correctly");
    //======================================================================================
    
    vTaskDelay(pdMS_TO_TICKS(ENCODER_TASK_STARTUP_DELAY));

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(encoderQueue, &encoderQueueMessage, 0))
        {
            ESP_LOGI(TAG, "Received message");
            switch(encoderQueueMessage.messageID)
            {
                case MESSAGE_ID_ENCODER_MODE_SINGLE:
                {
                    encoderVariationMode = ENCODER_MODE_SINGLE;
                }
                break;
                case MESSAGE_ID_ENCODER_MODE_CHUNK:
                {
                    encoderVariationMode = ENCODER_MODE_CHUNK;
                }
                break;
            }
        }

        // Encoder 
        int angleDifference = KY040EncoderGetAngleDifference(&ky040Encoder);
        switch (encoderVariationMode)
        {            
            case ENCODER_MODE_SINGLE:
            {
                if(angleDifference != 0)
                {
                    encoderMessageParams[0] = angleDifference;
                    SendMessage(managerQueue, SENDER_ID_ENCODER, DEVICE_ID_MANAGER, MESSAGE_ID_ENCODER_ANGLE_VARIATION, encoderMessageParams);

                    if(angleDifference > 0)                  
                        ESP_LOGI(TAG, "Triggered Inc-UP   Single-Mode %d", angleDifference);
                    else if(angleDifference < 0)                  
                        ESP_LOGI(TAG, "Triggered Inc-Down Single-Mode %d", angleDifference);   
                }
            }
            break;
        
            case ENCODER_MODE_CHUNK:
            {
                if(angleDifference > ENCODER_CHUNK_THRESHOLD || angleDifference < -ENCODER_CHUNK_THRESHOLD)
                {                    
                    encoderMessageParams[0] = angleDifference;
                    SendMessage(managerQueue, SENDER_ID_ENCODER, DEVICE_ID_MANAGER, MESSAGE_ID_ENCODER_ANGLE_VARIATION, encoderMessageParams);

                    if(angleDifference > ENCODER_CHUNK_THRESHOLD)                  
                        ESP_LOGI(TAG, "Triggered Inc-UP   Chunk-Mode %d", angleDifference);
                    else if(angleDifference < -ENCODER_CHUNK_THRESHOLD)                  
                        ESP_LOGI(TAG, "Triggered Inc-Down Chunk-Mode %d", angleDifference);   
                }
            }
            break;
        }     

        //Switch
        if(KY040SwitchGetActivation(&ky040Encoder))      
        {
            ESP_LOGI(TAG, "Triggered Switch");   
            SendMessage(managerQueue, SENDER_ID_ENCODER, DEVICE_ID_MANAGER, MESSAGE_ID_ENCODER_SWITCH_TRIGGER, NULL);
        }             
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(ENCODER_TASK_POLLING_RATE));
    }

    ESP_LOGE(TAG, "This section should not be reached");
    return;
}