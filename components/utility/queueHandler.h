#ifndef _QUEUE_HANDLER_H_
#define _QUEUE_HANDLER_H_

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

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define MESSAGE_PARAMS_LENGTH                       10
#define MESSAGE_PARAMS_SEND_TICKS                   1

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
#define MESSAGE_ID_ENCODER_SWITCH_TRIGGER           0x34

#define MESSAGE_ID_LED_CLEAR_LED                    0x41
#define MESSAGE_ID_LED_SET_EFFECT_RAINBOW           0x42
#define MESSAGE_ID_LED_SET_EFFECT_BREATH            0x43
#define MESSAGE_ID_LED_SET_EFFECT_SOLID             0x44
#define MESSAGE_ID_LED_SET_EFFECT_LOADING           0x45

//======================================================================================
/* 
*   Public variables & defines
*/
//======================================================================================

typedef struct{
    uint8_t senderID;
    uint8_t deviceID;
    uint8_t messageID;
    int params[10];
}queueMessage;

//======================================================================================
/* 
*   Public functions & routines
*/
//======================================================================================

void SendMessage(QueueHandle_t queue, uint8_t senderID, uint8_t deviceID, uint8_t messageID, int *params);
void CreateQueue(QueueHandle_t * queue, queueMessage * message, uint8_t size, char * taskTAG);

#endif /* _QUEUE_HANDLER_H_ */