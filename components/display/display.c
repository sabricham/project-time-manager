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
#include "bitmaps.h"
#include "sh1106.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Display"
QueueHandle_t display_queue = NULL;
queue_message display_queue_message;

SH1106_t display_handler;
uint8_t display_internal_state = 0;

/* Private functions & routines */

void print_page_mode_bytes(uint8_t *bitmap) 
{
    // There are 8 pages and each page has 128 bytes
    for (int page = 0; page < 8; ++page) {
        for (int column = 0; column < 64; ++column) {
            printf("0x%02X, ", bitmap[page * 64 + column]);
        }
        printf("\n");
    }
}

/* Public functions & routines */

void display_task()
{
    ESP_LOGW(TAG, "Starting task");
    uint8_t display_internal_state = DISPLAY_PAGE_IDLE;
    
    esp_task_wdt_add(NULL);
    start_queue(&display_queue, &display_queue_message, 10, TAG);

	ESP_LOGI(TAG, "I2C port is SDA:%d SCL:%d", I2C_PIN_SDA, I2C_PIN_SCL);
    i2c_master_init(&display_handler, I2C_PIN_SDA, I2C_PIN_SCL, -1);
    
	ESP_LOGI(TAG, "Panel is SH1106 %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
	sh1106_init(&display_handler, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	sh1106_contrast(&display_handler, 0xff);
	sh1106_clear_screen(&display_handler, false);
    
    uint8_t display_page[1024];
    memset(display_page, 0, sizeof(display_page));

    uint16_t display_horizontal_offset = 0;
    uint16_t display_digits_number = 0;

    ESP_LOGW(TAG, "Task started correctly");
    
    vTaskDelay(pdMS_TO_TICKS(100));

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(display_queue, &display_queue_message, 10))
        {
            ESP_LOGI(TAG, "Received message");
            switch(display_queue_message.sender_id)
            {
                case SENDER_ID_MANAGER:
                {
                    switch(display_queue_message.message_id)
                    {
                        case MESSAGE_ID_DISPLAY_PAGE_IDLE:
                        {
                            display_internal_state = DISPLAY_PAGE_IDLE;   

                            if(display_horizontal_offset + display_queue_message.params[0] >= 456-128)
                                display_horizontal_offset = 456-128;
                            else if(display_horizontal_offset + display_queue_message.params[0] <= 0)
                                display_horizontal_offset = 0;
                            else
                                display_horizontal_offset += display_queue_message.params[0];

                        }
                        break;
                        case MESSAGE_ID_DISPLAY_PAGE_DIGITS:
                        {
                            display_internal_state = DISPLAY_PAGE_DIGITS;   

                            display_digits_number = display_queue_message.params[0];
                            if (display_digits_number > 9999){
                                display_digits_number = display_digits_number % 10000; 
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

        switch(display_internal_state)
        {
            case DISPLAY_PAGE_IDLE:
            {                
                memset(display_page, 0, sizeof(display_page));
                sh1106_display_bitmap(&display_handler, screen_page_idle, display_horizontal_offset, 456);                
            }
            break;
            case DISPLAY_PAGE_DIGITS:
            {
                memcpy(display_page, screen_page_digits, sizeof(display_page));
                bitmap_sum(display_page, screen_digits[(display_digits_number/1000) % 10], 128, 64, 24, 64, 10, 0);
                bitmap_sum(display_page, screen_digits[(display_digits_number/100) % 10], 128, 64, 24, 64, 34, 0);
                bitmap_sum(display_page, screen_digits[(display_digits_number/10) % 10], 128, 64, 24, 64, 70, 0);
                bitmap_sum(display_page, screen_digits[display_digits_number % 10], 128, 64, 24, 64, 94, 0);
                sh1106_display_bitmap(&display_handler, display_page, 0, 128); 
            }
            break;
            case DISPLAY_PAGE_SETTINGS:
            {

            }
            break;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}