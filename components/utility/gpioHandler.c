#include "gpioHandler.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "GPIO Handler"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

//======================================================================================
/* 
*   Private functions & routines
*/
//======================================================================================

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
/*
*   Interface function to create a GPIO
*/
void GPIOCreate(uint8_t pin, gpio_int_type_t intrType, gpio_mode_t mode)
{
    gpio_config_t gpio_conf = {
        .intr_type = intrType,
        .mode = mode,
        .pin_bit_mask = (1ULL<<pin),
        .pull_up_en = 1
    };             
	ESP_ERROR_CHECK(gpio_config(&gpio_conf));
    ESP_LOGI(TAG, "New GPIO has been configured");
}

/*
*   Install ISR to enable the interrupt routine functions
*/
void GPIOInstallISR()
{
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
}

/*
*   Connect an interrupt routine to the GPIO
*/
void GPIOAddISR(uint8_t pin, gpio_isr_t isrRoutine, void *args)
{    
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin, isrRoutine, args));
}