#ifndef _ENCODER_H_
#define _ENCODER_H_

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

#include "manager.h"
#include "ky040.h"

#include "queueHandler.h"
#include "taskHandler.h"
#include "gpioHandler.h"
#include "timerHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define ENCODER_TASK_STARTUP_DELAY          100
#define ENCODER_TASK_POLLING_RATE           25

#define ENCODER_PIN_A                       0
#define ENCODER_PIN_B                       1
#define ENCODER_PIN_SWITCH                  2

#define ENCODER_ANGLE_INCREMENT             1

#define ENCODER_MODE_SINGLE                 0x01
#define ENCODER_MODE_CHUNK                  0x02

#define ENCODER_CHUNK_THRESHOLD             3

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

void EncoderTask();

#endif /* _ENCODER_H_ */