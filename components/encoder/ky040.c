#include "ky040.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "KY040"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

gptimer_handle_t debounceTimer = NULL;
bool debounceTimerActivated = false;

/* Private functions & routines */
/*-------------------------------------------------------------------------------------------------------*/
/*
*   Switch
*/
static void IRAM_ATTR KY040SwitchISR(void* arg) 
{
    KY040_t * encoder = (KY040_t *)arg;
    encoder->switchTrigger = true;
    return;
}

/*-------------------------------------------------------------------------------------------------------*/
/*
*   Encoder
*/
void KY040EncoderAddAngle(KY040_t * encoder)
{
    encoder->angularPosition += encoder->angleIncrement;
    return;
}

void KY040EncoderSubAngle(KY040_t * encoder)
{
    encoder->angularPosition -= encoder->angleIncrement;
    return;
}

void KY040EncoderCheckAngleVariation(KY040_t * encoder, uint8_t pinTrigger, int valuePinA, int valuePinB)
{
    if(pinTrigger == encoder->pinA)
    {
        if (valuePinA) {
            //Rising
            if(valuePinB){                
                KY040EncoderAddAngle(encoder);
            }
        } else {
            //Falling
            if(valuePinB){
                //KY040EncoderSubAngle(encoder);
            }
            else{
                //KY040EncoderAddAngle(encoder);
            }
        }
    }    
    else if(pinTrigger == encoder->pinB)
    {
        if (valuePinB) {
            //Rising
            if(!valuePinA){
                //KY040EncoderAddAngle(encoder); 
            }     
            else{
                KY040EncoderSubAngle(encoder);
            }
        } else {
            //Falling
            if(!valuePinA){
                //KY040EncoderSubAngle(encoder);
            }
            else{
                //KY040EncoderAddAngle(encoder);                
            }
        }
    } 
}

/*
    Interrupt routine for pin A slowed by a debounce timer, check if both are same level, then increment angle and activate timer
*/
static void IRAM_ATTR KY040EncoderISRPinA(void* arg) 
{
    KY040_t * encoder = (KY040_t *)arg;
    int valuePinA = gpio_get_level(encoder->pinA);
    int valuePinB = gpio_get_level(encoder->pinB);

    if(valuePinA == valuePinB)
    {
        if(!debounceTimerActivated)
        {  
            debounceTimerActivated = true;   
            KY040EncoderCheckAngleVariation(encoder, encoder->pinA, valuePinA, valuePinB);
            TimerStart(debounceTimer);   
        }
    }    
    return;
}

/*
    Interrupt routine for pin B slowed by a debounce timer, check if both are same level, then increment angle and activate timer
*/
static void IRAM_ATTR KY040EncoderISRPinB(void* arg) 
{
    KY040_t * encoder = (KY040_t *)arg;
    int valuePinA = gpio_get_level(encoder->pinA);
    int valuePinB = gpio_get_level(encoder->pinB);

    if(valuePinA == valuePinB)
    {
    if(!debounceTimerActivated)
        {
            debounceTimerActivated = true;    
            KY040EncoderCheckAngleVariation(encoder, encoder->pinB, valuePinA, valuePinB);   
            TimerStart(debounceTimer);  
        }
    }
    return;
}

/*
    Re-enables input pins interrupt and restores debounce timer to 0
*/
static void IRAM_ATTR KY040TimerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg)
{    
    TimerReset(timer, 0);
    debounceTimerActivated = false;
    return;
}

/* Public functions & routines */
/*-------------------------------------------------------------------------------------------------------*/
/*
*   KY040
*/
void KY040Init(KY040_t * encoder, uint8_t pinA, uint8_t pinB, uint8_t pinSwitch, uint8_t angleIncrement)
{
    // Decoder & switch pins settings
    GPIOCreate(pinA, GPIO_INTR_ANYEDGE, GPIO_MODE_INPUT);
    GPIOCreate(pinB, GPIO_INTR_ANYEDGE, GPIO_MODE_INPUT);
    GPIOCreate(pinSwitch, GPIO_INTR_ANYEDGE, GPIO_MODE_INPUT);

    GPIOInstallISR();

    GPIOAddISR(pinA, KY040EncoderISRPinA, (KY040_t *) encoder);
    GPIOAddISR(pinB, KY040EncoderISRPinB, (KY040_t *) encoder);
    GPIOAddISR(pinSwitch, KY040SwitchISR, (KY040_t *) encoder);    
    ESP_LOGI(TAG, "Encoder & switch gpio pins config successfully");

    // Debounce timer settings
    TimerCreate(&debounceTimer, 0, 1*1000*1000, false, GPTIMER_CLK_SRC_DEFAULT, GPTIMER_COUNT_UP, 500, 0, KY040TimerISR, (KY040_t *) encoder);
    ESP_LOGI(TAG, "Encoder debounce timer setup completed successfully");

    // Assign handles
    encoder->pinA = pinA;
    encoder->pinB = pinB;
    encoder->pinSwitch = pinSwitch;

    encoder->angleIncrement = angleIncrement;
    encoder->switchTrigger = false;
    return;
}

/*-------------------------------------------------------------------------------------------------------*/
/*
*   Encoder
*/
void KY040EncoderReset(KY040_t * encoder)
{
    if(encoder->angularPosition > KY040_ANGLE_RESET_LIMIT || encoder->angularPosition < -KY040_ANGLE_RESET_LIMIT)
    {        
        encoder->angularPosition = 0;
        encoder->angularPositionPrevious = 0;
    }
    else
    {        
        encoder->angularPositionPrevious = encoder->angularPosition;
    }
    return;
}

int KY040EncoderGetAngleDifference(KY040_t * encoder)
{     
    int difference = encoder->angularPosition - encoder->angularPositionPrevious;
    KY040EncoderReset(encoder);
    return difference;
}

/*-------------------------------------------------------------------------------------------------------*/
/*
*   Switch
*/
void KY040SwitchReset(KY040_t * encoder)
{
    encoder->switchTrigger = false;
    return;
}

bool KY040SwitchGetActivation(KY040_t * encoder)
{
    bool trigger = encoder->switchTrigger;
    KY040SwitchReset(encoder);
    return trigger;
}