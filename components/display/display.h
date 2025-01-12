#ifndef _DISPLAY_H_
#define _DISPLAY_H_

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

#include "bitmaps.h"
#include "sh1106.h"

#include "queueHandler.h"
#include "taskHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define DISPLAY_TASK_STARTUP_DELAY      100
#define DISPLAY_TASK_POLLING_RATE       25

#define I2C_PIN_SDA                     6
#define I2C_PIN_SCL                     7

#define DISPLAY_WIDTH                   128
#define DISPLAY_HEIGHT                  64

#define DISPLAY_PAGE_IDLE               0x01
#define DISPLAY_PAGE_DIGITS             0x02
#define DISPLAY_PAGE_SETTINGS           0x03

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

void DisplayTask();

#endif /* _DISPLAY_H_ */