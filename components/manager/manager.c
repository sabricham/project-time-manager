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
extern QueueHandle_t ledQueue;
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

//counting tens of milli seconds
gptimer_handle_t timerTensMilliSeconds = NULL;
int timerTimeCounter;
int timerTimerCounterPrevious;

//======================================================================================
/* 
*   Private functions & routines
*/
//======================================================================================
/*
*   Clear all modified variables
*/
void ResetManagerVariables()
{
    timerTimeSelected = 0;
    timerTimeSelectedPrevious = 0;
    timerTimeCounter = 0;
}

/*
*   Timer callback for every milli second passed decreases the counter
*/
static void IRAM_ATTR TimerTimeCounterCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg) 
{ 
    timerTimeCounter--;   
    return;
}

//======================================================================================
/*
*   FSM Startup state
*/
uint8_t ManagerStartup()
{ 
    //set encoder single mode
    SendMessage(encoderQueue, SENDER_ID_MANAGER, DEVICE_ID_ENCODER, MESSAGE_ID_ENCODER_MODE_SINGLE, NULL);
        
    //set screen time 00:00
    managerMessagesParams[0] = 0;
    SendMessage(displayQueue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, managerMessagesParams);

    //set led solid green effect
    managerMessagesParams[0] = 10;
    managerMessagesParams[1] = 255;
    managerMessagesParams[2] = 255;
    managerMessagesParams[3] = 255;
    SendMessage(ledQueue, SENDER_ID_MANAGER, DEVICE_ID_LED, MESSAGE_ID_LED_SET_EFFECT_SOLID, managerMessagesParams);

    vTaskDelay(pdMS_TO_TICKS(3000));

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
        ESP_LOGI(TAG, "Received message");

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
                    timerTimeCounter = timerTimeSelected * 100;     //tens of milliseconds * 100 makes a second
                    timerTimerCounterPrevious = 0;

                    TimerSet(timerTensMilliSeconds, 0);
                    TimerStart(timerTensMilliSeconds);
                    
                    //set led loading effect
                    managerMessagesParams[0] = 25;
                    managerMessagesParams[1] = 25;
                    managerMessagesParams[2] = 25;
                    managerMessagesParams[3] = 255;
                    managerMessagesParams[4] = 100;
                    SendMessage(ledQueue, SENDER_ID_MANAGER, DEVICE_ID_LED, MESSAGE_ID_LED_SET_EFFECT_LOADING, managerMessagesParams);

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
{
    // Get messages from other tasks
    if(xQueueReceive(managerQueue, &managerQueueMessage, 0))
    {
        ESP_LOGI(TAG, "Received message");
    }

    if(timerTimeCounter <= 0)
    {
        ESP_LOGI(TAG, "Time expired!");

        //reset all the needed for loop
        ResetManagerVariables();
        TimerReset(timerTensMilliSeconds, 0);

        //refresh image to 00:00
        managerMessagesParams[0] = 0;
        SendMessage(displayQueue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, managerMessagesParams);
        
        //set led solid green effect
        managerMessagesParams[0] = 10;
        managerMessagesParams[1] = 255;
        managerMessagesParams[2] = 255;
        managerMessagesParams[3] = 255;
        SendMessage(ledQueue, SENDER_ID_MANAGER, DEVICE_ID_LED, MESSAGE_ID_LED_SET_EFFECT_SOLID, managerMessagesParams);

        return managerStateIdle;
    }
    else if(timerTimeCounter != timerTimerCounterPrevious)   // /100 -> check every 1 s, /10 -> check every 100 ms, nothing -> check every 10 ms
    {
        timerTimerCounterPrevious = timerTimeCounter;
        ESP_LOGI(TAG, "timerTimerCounterPrevious: %d", timerTimerCounterPrevious);

        //refresh image to the time left
        managerMessagesParams[0] = ((timerTimerCounterPrevious / 100) / 60) * 100 + ((timerTimerCounterPrevious / 100) % 60);
        SendMessage(displayQueue, SENDER_ID_MANAGER, DEVICE_ID_DISPLAY, MESSAGE_ID_DISPLAY_PAGE_DIGITS, managerMessagesParams);
        
        //refresh led loading effect
        managerMessagesParams[0] = 10;
        managerMessagesParams[1] = 25;
        managerMessagesParams[2] = 25;
        managerMessagesParams[3] = 255;
        managerMessagesParams[4] = (timerTimerCounterPrevious * 100) / timerTimeSelected;
        SendMessage(ledQueue, SENDER_ID_MANAGER, DEVICE_ID_LED, MESSAGE_ID_LED_SET_EFFECT_LOADING, managerMessagesParams);        
    }
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
    CreateQueue(&managerQueue, &managerQueueMessage, 3, TAG);
    ResetManagerVariables();

    TimerCreate(&timerTensMilliSeconds, 1, 1*1000*1000, true, GPTIMER_CLK_SRC_DEFAULT, GPTIMER_COUNT_UP, 1000*10, 0, TimerTimeCounterCallback, NULL);
    managerState = managerStateStartup;
    
    vTaskDelay(pdMS_TO_TICKS(MANAGER_TASK_STARTUP_DELAY));

    ESP_LOGW(TAG, "Task started correctly");
    //======================================================================================

    while(1)
    {
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