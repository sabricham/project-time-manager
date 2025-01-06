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
#define SENDER_ID_LED                               0x40

/*  DEVICE_ID */
#define DEVICE_ID_DISPLAY                           0x11
#define DEVICE_ID_MANAGER                           0x21
#define DEVICE_ID_ENCODER                           0x31
#define DEVICE_ID_LED                               0x41

/*  MESSAGE_ID */
#define MESSAGE_ID_DISPLAY_PAGE_IDLE                0x11
#define MESSAGE_ID_DISPLAY_PAGE_DIGITS              0x12
#define MESSAGE_ID_DISPLAY_PAGE_SETTINGS            0x13

#define MESSAGE_ID_ENCODER_MODE_SINGLE              0x31
#define MESSAGE_ID_ENCODER_MODE_CHUNK               0x32
#define MESSAGE_ID_ENCODER_ANGLE_VARIATION          0x33

#define MESSAGE_ID_LED_CLEAR_LED                    0x41
#define MESSAGE_ID_LED_SET_EFFECT_RAINBOW           0x42
#define MESSAGE_ID_LED_SET_EFFECT_BREATH            0x43
#define MESSAGE_ID_LED_SET_EFFECT_SOLID             0x44
#define MESSAGE_ID_LED_SET_EFFECT_LOADING           0x45

/* Public functions & routines */
void start_queue(QueueHandle_t * queue, queue_message * message, uint8_t size, char * task_tag);

#endif /* _QUEUE_HANDLER_H_ */