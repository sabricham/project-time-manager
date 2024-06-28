#ifndef _TASK_HANDLER_H_
#define _TASK_HANDLER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

/* Components */

/* Public variables & defines */

/* Public functions & routines */
void start_task(TaskHandle_t (* task)(void *), char * task_name, uint16_t stack_size, uint8_t priority);

#endif /* _TASK_HANDLER_H_ */