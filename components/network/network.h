#ifndef _NETWORK_H_
#define _NETWORK_H_

//======================================================================================
/* 
*   Includes 
*/
//======================================================================================

#include <stdio.h>
#include <string.h>

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

#include "queueHandler.h"
#include "taskHandler.h"

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

#include <hap_fw_upgrade.h>
#include <iot_button.h>

#include <app_wifi.h>
#include <app_hap_setup_payload.h>

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

void fan_thread_entry(void *p);
void NetworkTask();

#endif /* _NETWORK_H_ */