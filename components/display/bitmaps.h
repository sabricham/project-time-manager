#ifndef _BITMAPS_H_
#define _BITMAPS_H_

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
#include "esp_log.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

// Single digit: 24x64 pixels 
extern uint8_t screen_digits[10][192]; 
// Idle page: 456x64 pixels 
extern uint8_t screen_page_idle[3648]; 
// Digits page: 128x64 pixels 
extern uint8_t screen_page_digits[1024];

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

void BitmapHorizontalToPageMode(uint8_t *horizontalModeBitmap, uint8_t *pageModeBitmap, uint8_t width, uint8_t height);
void BitmapSum(uint8_t * bitmapA, uint8_t * bitmapB, uint8_t widthA, uint8_t heightA, uint8_t widthB, uint8_t heightB, uint8_t posX, uint8_t posY);

#endif /* _BITMAPS_H_ */