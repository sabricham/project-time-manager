#ifndef _KY040_H_
#define _KY040_H_
/*
    Library for KY040 360 rotary encoder & switch support
*/

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
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "limits.h"

#include "gpioHandler.h"
#include "timerHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define KY040_ANGLE_RESET_LIMIT           100

//======================================================================================
/* 
*   Public variables & defines
*/
//======================================================================================

typedef struct{
    //physical connectors
    uint8_t pinA;
    uint8_t pinB;
    uint8_t pinSwitch;

    //encoder
    uint8_t angleIncrement;
    bool prevStatePinA;    
    bool prevStatePinB;    
    int angularPosition;   
    int angularPositionPrevious;

    //switch
    bool switchTrigger;
} KY040_t;

//======================================================================================
/* 
*   Public functions & routines
*/
//======================================================================================

void KY040Init(KY040_t * encoder, uint8_t pin_a, uint8_t pin_b, uint8_t pin_sw, uint8_t angle_increment);
int KY040EncoderGetAngleDifference(KY040_t * encoder);
bool KY040SwitchGetActivation(KY040_t * encoder);

#endif /* _KY040_H_ */