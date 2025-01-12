#ifndef _MANAGER_H_
#define _MANAGER_H_

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
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "display.h"
#include "encoder.h"
#include "led.h"

#include "queueHandler.h"
#include "taskHandler.h"
#include "timerHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define MANAGER_TASK_STARTUP_DELAY                      100
#define MANAGER_TASK_POLLING_RATE                       25

#define MANAGER_MAX_TIME_SELECTABLE                     60*60
#define MANAGER_MIN_TIME_SELECTABLE                     0
#define MANAGER_REGULAR_STEPS_TIME_SELECTION            5
#define MANAGER_LONG_STEPS_TIME_SELECTION               30
#define MANAGER_VERY_LONG_STEPS_TIME_SELECTION          60
#define MANAGER_STEPS_FIRST_TIME_SELECTION_THRESHOLD    90
#define MANAGER_STEPS_SECOND_TIME_SELECTION_THRESHOLD   360

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

void ManagerTask();

#endif /* _MANAGER_H_ */