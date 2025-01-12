#include "display.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Display"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

QueueHandle_t displayQueue = NULL;
queueMessage displayQueueMessage;
int displayMessageParams[MESSAGE_PARAMS_LENGTH];

SH1106_t displayHandler;
uint8_t displayInternalState = 0;

//======================================================================================
/* 
*   Private functions & routines
*/
//======================================================================================
/*
*   Print on screen an image in bitmap using page mode
*/
void print_page_mode_bytes(uint8_t *bitmap) 
{
    // There are 8 pages and each page has 128 bytes
    for (int page = 0; page < 8; ++page) 
    {
        for (int column = 0; column < 64; ++column) 
        {
            printf("0x%02X, ", bitmap[page * 64 + column]);
        }
        printf("\n");
    }
}

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
void DisplayTask()
{
    //======================================================================================
    ESP_LOGW(TAG, "Starting task");

    uint8_t displayInternalState = DISPLAY_PAGE_IDLE;
    
    esp_task_wdt_add(NULL);
    CreateQueue(&displayQueue, &displayQueueMessage, 3, TAG);

	ESP_LOGI(TAG, "I2C port is SDA:%d SCL:%d", I2C_PIN_SDA, I2C_PIN_SCL);
    i2c_master_init(&displayHandler, I2C_PIN_SDA, I2C_PIN_SCL, -1);
    
	ESP_LOGI(TAG, "Panel is SH1106 %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
	sh1106_init(&displayHandler, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	sh1106_contrast(&displayHandler, 0xff);
	sh1106_clear_screen(&displayHandler, false);
    
    uint8_t displayPage[1024];
    memset(displayPage, 0, sizeof(displayPage));

    uint16_t displayHorizontalOffset = 0;
    uint16_t displayDigitsNumber = 0;

    vTaskDelay(pdMS_TO_TICKS(DISPLAY_TASK_STARTUP_DELAY));

    ESP_LOGW(TAG, "Task started correctly");
    //======================================================================================

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(displayQueue, &displayQueueMessage, 10))
        {
            ESP_LOGI(TAG, "Received message");
            switch(displayQueueMessage.senderID)
            {
                case SENDER_ID_MANAGER:
                {
                    switch(displayQueueMessage.messageID)
                    {
                        case MESSAGE_ID_DISPLAY_PAGE_IDLE:
                        {
                            displayInternalState = DISPLAY_PAGE_IDLE;   

                            if(displayHorizontalOffset + displayQueueMessage.params[0] >= 456-128)
                                displayHorizontalOffset = 456-128;
                            else if(displayHorizontalOffset + displayQueueMessage.params[0] <= 0)
                                displayHorizontalOffset = 0;
                            else
                                displayHorizontalOffset += displayQueueMessage.params[0];

                        }
                        break;
                        case MESSAGE_ID_DISPLAY_PAGE_DIGITS:
                        {
                            displayInternalState = DISPLAY_PAGE_DIGITS;   

                            displayDigitsNumber = displayQueueMessage.params[0];
                            if (displayDigitsNumber > 9999){
                                displayDigitsNumber = displayDigitsNumber % 10000; 
                            }                                                                             
                        }
                        break;
                        case MESSAGE_ID_DISPLAY_PAGE_SETTINGS:
                        {

                        }
                        break;
                    }
                }
                break;
            }
        }

        switch(displayInternalState)
        {
            case DISPLAY_PAGE_IDLE:
            {                
                memset(displayPage, 0, sizeof(displayPage));
                sh1106_display_bitmap(&displayHandler, screen_page_idle, displayHorizontalOffset, 456);                
            }
            break;
            case DISPLAY_PAGE_DIGITS:
            {
                memcpy(displayPage, screen_page_digits, sizeof(displayPage));
                BitmapSum(displayPage, screen_digits[(displayDigitsNumber/1000) % 10], 128, 64, 24, 64, 10, 0);
                BitmapSum(displayPage, screen_digits[(displayDigitsNumber/100) % 10], 128, 64, 24, 64, 34, 0);
                BitmapSum(displayPage, screen_digits[(displayDigitsNumber/10) % 10], 128, 64, 24, 64, 70, 0);
                BitmapSum(displayPage, screen_digits[displayDigitsNumber % 10], 128, 64, 24, 64, 94, 0);
                sh1106_display_bitmap(&displayHandler, displayPage, 0, 128); 
            }
            break;
            case DISPLAY_PAGE_SETTINGS:
            {

            }
            break;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_TASK_POLLING_RATE));
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}