#include "timerHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Timer Handler"

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
*   Interface function to create a timer and enable it
*/
void TimerCreate(gptimer_handle_t *timerHandle, int priority, gptimer_clock_source_t clockSource, gptimer_count_direction_t countDirection, uint64_t triggerValue, uint64_t startingValue, gptimer_alarm_cb_t timerCallback, void * args)
{
    // Debounce timer settings
    gptimer_config_t timer_config = {
        .intr_priority = priority,
        .clk_src = clockSource,
        .direction = countDirection,
        .resolution_hz = 1 * 1000 * 1000, 
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, timerHandle));
    gptimer_alarm_config_t timer_alarm_config = {
        .flags.auto_reload_on_alarm = 0,
        .alarm_count = triggerValue
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(*timerHandle, &timer_alarm_config));
    ESP_ERROR_CHECK(gptimer_set_raw_count(*timerHandle, startingValue));
    gptimer_event_callbacks_t timer_event_callbacks = {
        .on_alarm = timerCallback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(*timerHandle, &timer_event_callbacks, args));
    ESP_ERROR_CHECK(gptimer_enable(*timerHandle));

    if(timerHandle == NULL)
    {
        ESP_LOGE(TAG, "TimerCreate error invalid timer, not initialized");
    }
    else
    {
        ESP_LOGI(TAG, "New timer has been configured");
    }
}

/*
*   Enable the timer
*/
void TimerEnable(gptimer_handle_t timerHandle)
{
    ESP_ERROR_CHECK(gptimer_enable(timerHandle));
}

/*
*   Disable the timer
*/
void TimerDisable(gptimer_handle_t timerHandle)
{
    ESP_ERROR_CHECK(gptimer_disable(timerHandle));
}

/*
*   Stop and set the starting value of the timer
*/
void TimerReset(gptimer_handle_t timerHandle, uint64_t startingValue)
{
    ESP_ERROR_CHECK(gptimer_stop(timerHandle));
    ESP_ERROR_CHECK(gptimer_set_raw_count(timerHandle, startingValue));
}

/*
*   Set the starting value of the timer
*/
void TimerSet(gptimer_handle_t timerHandle, uint64_t startingValue)
{
    ESP_ERROR_CHECK(gptimer_set_raw_count(timerHandle, startingValue));
}

/*
*   Start the timer
*/
void TimerStart(gptimer_handle_t timerHandle)
{
    ESP_ERROR_CHECK(gptimer_start(timerHandle));
}

/*
*   Stop the timer
*/
void TimerStop(gptimer_handle_t timerHandle)
{
    ESP_ERROR_CHECK(gptimer_stop(timerHandle));
}