#ifndef _ENCODER_H_
#define _ENCODER_H_

/* Components */


/* Public variables & defines */
#define ENCODER_PIN_A                       0
#define ENCODER_PIN_B                       1
#define ENCODER_ANGLE_INCREMENT             1

#define ENCODER_POLLING_RATE                30  //ms

#define ENCODER_MODE_SINGLE                 0x01
#define ENCODER_MODE_CHUNK                  0x02

#define ENCODER_CHUNK_THRESHOLD             3

/* Public functions & routines */
void encoder_task();

#endif /* _ENCODER_H_ */