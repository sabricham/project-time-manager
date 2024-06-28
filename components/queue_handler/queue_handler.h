#ifndef _QUEUE_HANDLER_H_
#define _QUEUE_HANDLER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

/* Components */

/* Public variables & defines */
typedef struct{
    uint8_t sender_id;
    uint8_t device_id;
    uint8_t message_id;
    int data[8];
}queue_message;

/*  SENDER_ID */
#define SENDER_ID_DISPLAY                           0x10
#define SENDER_ID_MANAGER                           0x20
#define SENDER_ID_ENCODER                           0x30

/*  DEVICE_ID */
#define DEVICE_ID_DISPLAY                           0x11
#define DEVICE_ID_MANAGER                           0x21
#define DEVICE_ID_ENCODER                           0x31

/*  MESSAGE_ID */
#define MESSAGE_ID_DISPLAY_PAGE_IDLE                0x11
#define MESSAGE_ID_DISPLAY_PAGE_DIGITS              0x12
#define MESSAGE_ID_DISPLAY_PAGE_SETTINGS            0x13

#define MESSAGE_ID_ENCODER_MODE_SINGLE              0x31
#define MESSAGE_ID_ENCODER_MODE_CHUNK               0x32
#define MESSAGE_ID_ENCODER_ANGLE_VARIATION          0x33

/* Public functions & routines */
void start_queue(QueueHandle_t * queue, queue_message * message, uint8_t size, char * task_tag);

#endif /* _QUEUE_HANDLER_H_ */