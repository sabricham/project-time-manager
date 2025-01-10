#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

/* Components */
#include "manager.h"
#include "display.h"
#include "encoder.h"

#include "queue_handler.h"
#include "task_handler.h"

/* Private variables & defines */
#define TAG "Manager"
extern QueueHandle_t display_queue;
extern QueueHandle_t encoder_queue;
QueueHandle_t manager_queue = NULL;
queue_message manager_queue_message;
int manager_messages_params[MESSAGE_PARAMS_LENGTH];

enum manager_internal_state{
    manager_state_startup,
    manager_state_idle,
    manager_state_timer,
    manager_state_stopwatch,
    manager_state_pomodoro,
    manager_state_settings
};
uint8_t manager_state = manager_state_idle;

TickType_t timer_seconds_counter = 0;

/*-----------------------------------------------------*/
#define MAX_TIME_SELECTABLE                     60*60
#define MIN_TIME_SELECTABLE                     0
#define REGULAR_STEPS_TIME_SELECTION            5
#define LONG_STEPS_TIME_SELECTION               30
#define VERY_LONG_STEPS_TIME_SELECTION          60
#define STEPS_FIRST_TIME_SELECTION_THRESHOLD    90
#define STEPS_SECOND_TIME_SELECTION_THRESHOLD   360

uint16_t timer_time_selected;
uint16_t timer_time_selected_previous;
uint16_t timer_time_passed;

/* Private functions & routines */
void reset_variables()
{
    timer_time_selected = 0;
    timer_time_selected_previous = 0;
    timer_time_passed = 0;
}

uint8_t manager_startup()
{ 
    // set encoder single mode
    send_message(encoder_queue, SENDER_ID_MANAGER, DEVICE_ID_ENCODER, MESSAGE_ID_ENCODER_MODE_SINGLE, NULL);
        
    //set screen time 00:00
    manager_messages_params[0] = 0;
    send_message(display_queue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, manager_messages_params);

    return manager_state_idle;
}

uint8_t manager_idle()
{    
    queue_message rx_mess; 

    if(xQueueReceive(manager_queue, &rx_mess, 0))
    {
        if(rx_mess.sender_id == SENDER_ID_ENCODER)
        {
            switch(rx_mess.message_id)
            {
                case MESSAGE_ID_ENCODER_ANGLE_VARIATION:
                {
                    //within max min range of time selectability
                    if(timer_time_selected + rx_mess.params[0] <= MAX_TIME_SELECTABLE || timer_time_selected + rx_mess.params[0] >= MIN_TIME_SELECTABLE)
                    {
                        //under first threshold
                        if(timer_time_selected < STEPS_FIRST_TIME_SELECTION_THRESHOLD) 
                        {
                            //going over threshold
                            if(timer_time_selected + rx_mess.params[0] * REGULAR_STEPS_TIME_SELECTION >= STEPS_FIRST_TIME_SELECTION_THRESHOLD) 
                            {
                                ESP_LOGI(TAG, "going over first threshold");
                                timer_time_selected = STEPS_FIRST_TIME_SELECTION_THRESHOLD;
                            }
                            //trying to go under min
                            else if(timer_time_selected + rx_mess.params[0] * REGULAR_STEPS_TIME_SELECTION < MIN_TIME_SELECTABLE)
                            {
                                ESP_LOGI(TAG, "trying to go under min");
                                timer_time_selected = MIN_TIME_SELECTABLE;
                            }  
                            //staying under threshold 
                            else    
                            {
                                ESP_LOGI(TAG, "staying under first threshold ");
                                timer_time_selected += rx_mess.params[0] * REGULAR_STEPS_TIME_SELECTION;
                            }
                        }
                        //over first threshold
                        else if(timer_time_selected < STEPS_SECOND_TIME_SELECTION_THRESHOLD)
                        {
                            //going over second threshold
                            if(timer_time_selected + rx_mess.params[0] * LONG_STEPS_TIME_SELECTION >= STEPS_SECOND_TIME_SELECTION_THRESHOLD)
                            {                                
                                ESP_LOGI(TAG, "going over second threshold");
                                timer_time_selected = STEPS_SECOND_TIME_SELECTION_THRESHOLD;
                            }
                            //going under first threshold
                            else if(timer_time_selected + rx_mess.params[0] * LONG_STEPS_TIME_SELECTION < STEPS_FIRST_TIME_SELECTION_THRESHOLD) 
                            {
                                ESP_LOGI(TAG, "going under first threshold");
                                timer_time_selected = STEPS_FIRST_TIME_SELECTION_THRESHOLD - REGULAR_STEPS_TIME_SELECTION;
                            }                             
                            //staying over first threshold 
                            else
                            {
                                ESP_LOGI(TAG, "staying over first threshold & under second threshold");
                                timer_time_selected += rx_mess.params[0] * LONG_STEPS_TIME_SELECTION;
                            }                            
                        }
                        //over second threshold
                        else
                        {   
                            //trying to go over max
                            if(timer_time_selected + rx_mess.params[0] * VERY_LONG_STEPS_TIME_SELECTION > MAX_TIME_SELECTABLE)
                            {
                                ESP_LOGI(TAG, "trying to go over max");
                                timer_time_selected = MAX_TIME_SELECTABLE;
                            }
                            //going under first threshold
                            else if(timer_time_selected + rx_mess.params[0] * VERY_LONG_STEPS_TIME_SELECTION < STEPS_SECOND_TIME_SELECTION_THRESHOLD) 
                            {
                                ESP_LOGI(TAG, "going under second threshold");
                                timer_time_selected = STEPS_SECOND_TIME_SELECTION_THRESHOLD - LONG_STEPS_TIME_SELECTION;
                            }  
                            //staying over second threshold 
                            else
                            {
                                ESP_LOGI(TAG, "staying over second threshold");
                                timer_time_selected += rx_mess.params[0] * VERY_LONG_STEPS_TIME_SELECTION;
                            }  
                        }
                    }               
                }
                break;
            }
        }
    } 

    if(timer_time_selected_previous != timer_time_selected)
    {
        timer_time_selected_previous = timer_time_selected;
        
        //refresh screen with new selected time
        manager_messages_params[0] = (timer_time_selected/60)*100 + (timer_time_selected%60);
        send_message(display_queue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, manager_messages_params);
    }

    return manager_state_idle;
}

uint8_t manager_timer()
{
    return manager_state_timer;
}

uint8_t manager_stopwatch()
{
    return manager_state_stopwatch;    
}

uint8_t manager_pomodoro()
{
    return manager_state_pomodoro;
}

uint8_t manager_settings()
{
    return manager_state_settings;
}

/* Public functions & routines */
void manager_task()
{
    ESP_LOGW(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    start_queue(&manager_queue, &manager_queue_message, 10, TAG);
    manager_state = manager_state_startup;
    reset_variables();

    ESP_LOGW(TAG, "Task started correctly");
    
    vTaskDelay(pdMS_TO_TICKS(100));

    while(1)
    {
        /*
        // Get messages from other tasks
        if(xQueueReceive(manager_queue, &manager_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }
        */
        
        switch(manager_state)
        {
            case manager_state_startup:
            {
                manager_state = manager_startup();
            }
            break;
            case manager_state_idle:
            {
                manager_state = manager_idle();
            }
            break;
            case manager_state_timer:
            {
                manager_state = manager_timer();
            }
            break;
            case manager_state_stopwatch:
            {
                manager_state = manager_stopwatch();
            }
            break;
            case manager_state_pomodoro:
            {
                manager_state = manager_pomodoro();
            }
            break;
            case manager_state_settings:
            {
                manager_state = manager_settings();
            }
            break;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}