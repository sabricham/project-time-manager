#ifndef _TIMER_H_
#define _TIMER_H_

//======================================================================================
/* 
*   Includes 
*/
//======================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//======================================================================================
/* 
*   Components
*/
//======================================================================================

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"

//======================================================================================
/* 
*   Macros
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

void TimerCreate(gptimer_handle_t *timerHandle, 
    int priority, 
    uint64_t resolution, 
    bool autoReload, 
    gptimer_clock_source_t clockSource, 
    gptimer_count_direction_t countDirection, 
    uint64_t triggerValue, 
    uint64_t startingValue, 
    gptimer_alarm_cb_t timerCallback, 
    void * args);
void TimerEnable(gptimer_handle_t timerHandle);
void TimerDisable(gptimer_handle_t timerHandle);
void TimerReset(gptimer_handle_t timerHandle, uint64_t startingValue);
void TimerSet(gptimer_handle_t timerHandle, uint64_t startingValue);
uint64_t TimerGet(gptimer_handle_t timerHandle);
void TimerStart(gptimer_handle_t timerHandle);
void TimerStop(gptimer_handle_t timerHandle);

#endif /* _TIMER_H_ */