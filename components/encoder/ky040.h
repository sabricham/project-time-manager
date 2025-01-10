#ifndef _KY040_H_
#define _KY040_H_

/*
    Library for KY040 360 rotary encoder support
*/

#define KY040_ANGLE_RESET_LIMIT           100

/* Components */


/* Public variables & defines */
typedef struct{
    uint8_t pin_a;
    uint8_t pin_b;
    uint8_t angle_increment;
    bool prev_pin_a_state;    
    bool prev_pin_b_state;    
    int angular_position;   
    int angular_position_previous;
} KY040_t;

/* Public functions & routines */
void debug();

void ky040_init(KY040_t * encoder, uint8_t pin_a, uint8_t pin_b, uint8_t angle_increment);
void ky040_reset(KY040_t * encoder);
int ky040_get_angle_difference(KY040_t * encoder);

#endif /* _KY040_H_ */