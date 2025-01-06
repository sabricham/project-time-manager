#ifndef _LED_H_
#define _LED_H_

/* Components */


/* Public variables & defines */
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

#define LED_STRIP_WS2812B_DATA_PIN          5
#define LED_STRIP_WS2812B_NUM_LEDS          18

/* Public functions & routines */
void led_task();

#endif /* _LED_H_ */