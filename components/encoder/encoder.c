#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

/* Components */
#include "encoder.h"
#include "ky040.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Encoder"
QueueHandle_t encoder_queue = NULL;
extern QueueHandle_t manager_queue;
queue_message encoder_queue_message;

uint8_t encoder_variation_mode = 0;

/* Public functions & routines */
QueueHandle_t queue;

void encoder_task()
{    
    ESP_LOGW(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    start_queue(&encoder_queue, &encoder_queue_message, 10, TAG);
    encoder_variation_mode = ENCODER_MODE_SINGLE;
    KY040_t ky040_encoder;
    ky040_init(&ky040_encoder, ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_ANGLE_INCREMENT);

    ESP_LOGW(TAG, "Task started correctly");

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(encoder_queue, &encoder_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
            switch(encoder_queue_message.message_id)
            {
                case MESSAGE_ID_ENCODER_MODE_SINGLE:
                {
                    encoder_variation_mode = ENCODER_MODE_SINGLE;
                }
                break;
                case MESSAGE_ID_ENCODER_MODE_CHUNK:
                {
                    encoder_variation_mode = ENCODER_MODE_CHUNK;
                }
                break;
            }
        }

        int angle_variation = ky040_get_angle_variation(&ky040_encoder);
        switch (encoder_variation_mode)
        {            
            case ENCODER_MODE_SINGLE:
            {
                if(angle_variation != 0)
                {
                    queue_message tx_mess;
                    
                    memset(&tx_mess, 0, sizeof(queue_message));
                    tx_mess.sender_id = SENDER_ID_ENCODER;
                    tx_mess.device_id = DEVICE_ID_MANAGER;
                    tx_mess.message_id = MESSAGE_ID_ENCODER_ANGLE_VARIATION;
                    tx_mess.data[0] = angle_variation;
                    while(manager_queue == NULL)
                        vTaskDelay(100);
                    xQueueSend(manager_queue, &tx_mess, 10);

                    ky040_reset(&ky040_encoder);
                    if(angle_variation > 0)                  
                        ESP_LOGI(TAG, "Triggered Inc-UP   Single-Mode %d", angle_variation);
                    else if(angle_variation < 0)                  
                        ESP_LOGI(TAG, "Triggered Inc-Down Single-Mode %d", angle_variation);   
                }
            }
            break;
        
            case ENCODER_MODE_CHUNK:
            {
                if(angle_variation > ENCODER_CHUNK_THRESHOLD || angle_variation < -ENCODER_CHUNK_THRESHOLD)
                {
                    queue_message tx_mess;

                    memset(&tx_mess, 0, sizeof(queue_message));
                    tx_mess.sender_id = SENDER_ID_ENCODER;
                    tx_mess.device_id = DEVICE_ID_MANAGER;
                    tx_mess.message_id = MESSAGE_ID_ENCODER_ANGLE_VARIATION;
                    tx_mess.data[0] = angle_variation;
                    while(manager_queue == NULL)
                        vTaskDelay(100);
                    xQueueSend(manager_queue, &tx_mess, 10);

                    ky040_reset(&ky040_encoder);
                    if(angle_variation > ENCODER_CHUNK_THRESHOLD)                  
                        ESP_LOGI(TAG, "Triggered Inc-UP   Chunk-Mode %d", angle_variation);
                    else if(angle_variation < -ENCODER_CHUNK_THRESHOLD)                  
                        ESP_LOGI(TAG, "Triggered Inc-Down Chunk-Mode %d", angle_variation);   
                }
            }
            break;
        }
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(ENCODER_POLLING_RATE));
    }

    ESP_LOGE(TAG, "This section should not be reached");
    return;
}