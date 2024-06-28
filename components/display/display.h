#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/* Components */

/* Public variables & defines */

#define I2C_PIN_SDA                     6
#define I2C_PIN_SCL                     7

#define DISPLAY_WIDTH                   128
#define DISPLAY_HEIGHT                  64

#define DISPLAY_PAGE_IDLE               0x01
#define DISPLAY_PAGE_DIGITS             0x02
#define DISPLAY_PAGE_SETTINGS           0x03

/* Public functions & routines */

void display_task();

#endif /* _DISPLAY_H_ */