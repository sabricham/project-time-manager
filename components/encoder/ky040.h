#ifndef _KY040_H_
#define _KY040_H_

/*
    Library for KY040 360 rotary encoder support
*/


/* Components */


/* Public variables & defines */
typedef struct{
    uint8_t _pin_a;
    uint8_t _pin_b;
    uint8_t _angle_increment;
    bool _prev_pin_a_state;    
    bool _prev_pin_b_state;    
    int _angle_variation;
} KY040_t;

/* Public functions & routines */
void debug();

void ky040_init(KY040_t * encoder, uint8_t pin_a, uint8_t pin_b, uint8_t angle_increment);
void ky040_reset(KY040_t * encoder);
int ky040_get_angle_variation(KY040_t * encoder);

#endif /* _KY040_H_ */