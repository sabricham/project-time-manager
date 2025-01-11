#include "manager.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Manager"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

extern QueueHandle_t displayQueue;
extern QueueHandle_t encoderQueue;
QueueHandle_t managerQueue = NULL;
queueMessage managerQueueMessage;
int managerMessagesParams[MESSAGE_PARAMS_LENGTH];

enum managerInternalState{
    managerStateStartup,
    managerStateIdle,
    managerStateTimer,
    managerStateStopwatch,
    managerStatePomodoro,
    managerStateSettings
};
uint8_t managerState = managerStateStartup;

uint16_t timerTimeSelected;
uint16_t timerTimeSelectedPrevious;
gptimer_handle_t timerSeconds = NULL;
uint16_t timerTimeCounter;
bool timerTimeExpired;

//======================================================================================
/* 
*   Private functions & routines
*/
//======================================================================================
/*
*   Clear all modified variables
*/
void ResetVariables()
{
    timerTimeSelected = 0;
    timerTimeSelectedPrevious = 0;
    timerTimeExpired = false;
}

/*
*   Timer callback when finished counting
*/
void TimerTimeCounterCallback(void * args) 
{ 
    // This function will be called when the timer expires
    ESP_LOGI(TAG, "Time counter, timer expired!");    
    timerTimeExpired = true;
}

//======================================================================================
/*
*   FSM Startup state
*/
uint8_t ManagerStartup()
{ 
    // set encoder single mode
    SendMessage(encoderQueue, SENDER_ID_MANAGER, DEVICE_ID_ENCODER, MESSAGE_ID_ENCODER_MODE_SINGLE, NULL);
        
    //set screen time 00:00
    managerMessagesParams[0] = 0;
    SendMessage(displayQueue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, managerMessagesParams);

    return managerStateIdle;
}

/*
*   FSM Idle state
*/
uint8_t ManagerIdle()
{   
    queueMessage messageRX; 
    
    if(xQueueReceive(managerQueue, &messageRX, 0))
    {
        if(messageRX.senderID == SENDER_ID_ENCODER)
        {
            switch(messageRX.messageID)
            {
                case MESSAGE_ID_ENCODER_ANGLE_VARIATION:    //Encoder angle has been changed
                {
                    // Selected time has been changed
                    if(timerTimeSelected + messageRX.params[0] <= MANAGER_MAX_TIME_SELECTABLE || timerTimeSelected + messageRX.params[0] >= MANAGER_MIN_TIME_SELECTABLE)
                    {
                        //under first threshold
                        if(timerTimeSelected < MANAGER_STEPS_FIRST_TIME_SELECTION_THRESHOLD) 
                        {
                            //going over threshold
                            if(timerTimeSelected + messageRX.params[0] * MANAGER_REGULAR_STEPS_TIME_SELECTION >= MANAGER_STEPS_FIRST_TIME_SELECTION_THRESHOLD) 
                            {
                                ESP_LOGI(TAG, "going over first threshold");
                                timerTimeSelected = MANAGER_STEPS_FIRST_TIME_SELECTION_THRESHOLD;
                            }
                            //trying to go under min
                            else if(timerTimeSelected + messageRX.params[0] * MANAGER_REGULAR_STEPS_TIME_SELECTION < MANAGER_MIN_TIME_SELECTABLE)
                            {
                                ESP_LOGI(TAG, "trying to go under min");
                                timerTimeSelected = MANAGER_MIN_TIME_SELECTABLE;
                            }  
                            //staying under threshold 
                            else    
                            {
                                ESP_LOGI(TAG, "staying under first threshold ");
                                timerTimeSelected += messageRX.params[0] * MANAGER_REGULAR_STEPS_TIME_SELECTION;
                            }
                        }
                        //over first threshold
                        else if(timerTimeSelected < MANAGER_STEPS_SECOND_TIME_SELECTION_THRESHOLD)
                        {
                            //going over second threshold
                            if(timerTimeSelected + messageRX.params[0] * MANAGER_LONG_STEPS_TIME_SELECTION >= MANAGER_STEPS_SECOND_TIME_SELECTION_THRESHOLD)
                            {                                
                                ESP_LOGI(TAG, "going over second threshold");
                                timerTimeSelected = MANAGER_STEPS_SECOND_TIME_SELECTION_THRESHOLD;
                            }
                            //going under first threshold
                            else if(timerTimeSelected + messageRX.params[0] * MANAGER_LONG_STEPS_TIME_SELECTION < MANAGER_STEPS_FIRST_TIME_SELECTION_THRESHOLD) 
                            {
                                ESP_LOGI(TAG, "going under first threshold");
                                timerTimeSelected = MANAGER_STEPS_FIRST_TIME_SELECTION_THRESHOLD - MANAGER_REGULAR_STEPS_TIME_SELECTION;
                            }                             
                            //staying over first threshold 
                            else
                            {
                                ESP_LOGI(TAG, "staying over first threshold & under second threshold");
                                timerTimeSelected += messageRX.params[0] * MANAGER_LONG_STEPS_TIME_SELECTION;
                            }                            
                        }
                        //over second threshold
                        else
                        {   
                            //trying to go over max
                            if(timerTimeSelected + messageRX.params[0] * MANAGER_VERY_LONG_STEPS_TIME_SELECTION > MANAGER_MAX_TIME_SELECTABLE)
                            {
                                ESP_LOGI(TAG, "trying to go over max");
                                timerTimeSelected = MANAGER_MAX_TIME_SELECTABLE;
                            }
                            //going under first threshold
                            else if(timerTimeSelected + messageRX.params[0] * MANAGER_VERY_LONG_STEPS_TIME_SELECTION < MANAGER_STEPS_SECOND_TIME_SELECTION_THRESHOLD) 
                            {
                                ESP_LOGI(TAG, "going under second threshold");
                                timerTimeSelected = MANAGER_STEPS_SECOND_TIME_SELECTION_THRESHOLD - MANAGER_LONG_STEPS_TIME_SELECTION;
                            }  
                            //staying over second threshold 
                            else
                            {
                                ESP_LOGI(TAG, "staying over second threshold");
                                timerTimeSelected += messageRX.params[0] * MANAGER_VERY_LONG_STEPS_TIME_SELECTION;
                            }  
                        }
                    }  
                    
                    // Refresh screen
                    if(timerTimeSelectedPrevious != timerTimeSelected)
                    {
                        timerTimeSelectedPrevious = timerTimeSelected;
                        
                        managerMessagesParams[0] = (timerTimeSelected / 60) * 100 + (timerTimeSelected % 60);
                        SendMessage(displayQueue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, managerMessagesParams);
                    }             
                }
                break;
                case MESSAGE_ID_ENCODER_SWITCH_TRIGGER:    //Encoder switch has been triggered
                {   
                    // Start the timer and change state
                    ESP_LOGI(TAG, "Switching fsm state to Timer");
                    timerTimeSelectedPrevious = timerTimeSelected; 

                    return managerStateTimer;            
                }
                break;
            }
        }
    } 

    return managerStateIdle;
}

/*
*   FSM Timer state
*/
uint8_t ManagerTimer()
{/*
    // Check timer expired
    if(timer_time_completed)
    {
        reset_variables();
        return manager_state_timer;
    }

    // Refresh screen
    if((xTimerGetExpiryTime(timer_time_counter) - xTaskGetTickCount())/100 != timer_time_counter_previous)
    {
        timer_time_counter_previous = (xTimerGetExpiryTime(timer_time_counter) - xTaskGetTickCount()) / 100;

        managerMessagesParams[0] = (timer_time_counter_previous/60)*100 + (timer_time_counter_previous%60);
        send_message(displayQueue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, managerMessagesParams);
    }

    // Log
    char float_str[20]; 
    sprintf(float_str, "%.2ld", (xTimerGetExpiryTime(timer_time_counter) - xTaskGetTickCount()) / 100);

    ESP_LOGI(TAG, "timer_counter: %s, casted_timer_counter: %ld", float_str, timer_time_counter_previous);*/
    
    return managerStateTimer;
}

/*
*   FSM Stopwatch state
*/
uint8_t ManagerStopwatch()
{
    return managerStateStopwatch;    
}

/*
*   FSM Pomodoro state
*/
uint8_t ManagerPomodoro()
{
    return managerStatePomodoro;
}

/*
*   FSM Settings state
*/
uint8_t ManagerSettings()
{
    return managerStateSettings;
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
void ManagerTask()
{
    //======================================================================================
    ESP_LOGW(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    CreateQueue(&managerQueue, &managerQueueMessage, 10, TAG);
    ResetVariables();

    TimerCreate(&timerSeconds, 1, GPTIMER_CLK_SRC_DEFAULT, GPTIMER_COUNT_UP, 500, 0, TimerTimeCounterCallback, NULL);
    managerState = managerStateStartup;

    ESP_LOGW(TAG, "Task started correctly");
    //======================================================================================
    
    vTaskDelay(pdMS_TO_TICKS(MANAGER_TASK_STARTUP_DELAY));

    while(1)
    {
        /*
        // Get messages from other tasks
        if(xQueueReceive(manager_queue, &manager_queue_message, 0))
        {
            ESP_LOGI(TAG, "Received message");
        }
        */
        
        switch(managerState)
        {
            case managerStateStartup:
            {
                managerState = ManagerStartup();
            }
            break;
            case managerStateIdle:
            {
                managerState = ManagerIdle();
            }
            break;
            case managerStateTimer:
            {
                managerState = ManagerTimer();
            }
            break;
            case managerStateStopwatch:
            {
                managerState = ManagerStopwatch();
            }
            break;
            case managerStatePomodoro:
            {
                managerState = ManagerPomodoro();
            }
            break;
            case managerStateSettings:
            {
                managerState = ManagerSettings();
            }
            break;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(MANAGER_TASK_POLLING_RATE));
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}