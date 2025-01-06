#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/pulse_cnt.h"
#include "esp_system.h"
#include "esp_log.h"
#include "limits.h"

/* Components */
#include "ky040.h"

/* Private variables & defines */
#define TAG "KY040-Encoder"
gptimer_handle_t debounce_timer = NULL;

#define EXAMPLE_EC11_GPIO_A 0
#define EXAMPLE_EC11_GPIO_B 1

bool debounce_timer_activated = false;

/* Private functions & routines */
void ky040_add_angle_variation(KY040_t * encoder)
{
    encoder->_angle_variation += encoder->_angle_increment;
    return;
}

void ky040_sub_angle_variation(KY040_t * encoder)
{
    encoder->_angle_variation -= encoder->_angle_increment;
    return;
}

void ky040_check_angle_variation(KY040_t * encoder, uint8_t pin_trigger)
{
    int level_a = gpio_get_level(encoder->_pin_a);
    int level_b = gpio_get_level(encoder->_pin_b);

    if(pin_trigger == encoder->_pin_a)
    {
        if (level_a) {
            //Rising
            if(level_b)
                ky040_add_angle_variation(encoder);
            else
                ky040_sub_angle_variation(encoder);         
        } else {
            //Falling
            if(level_b)
                ky040_sub_angle_variation(encoder);
            else
                ky040_add_angle_variation(encoder);
        }
    }    
    else if(pin_trigger == encoder->_pin_b)
    {
        if (level_b) {
            //Rising
            if(!level_a)
                ky040_add_angle_variation(encoder);
            else
                ky040_sub_angle_variation(encoder);
        } else {
            //Falling
            if(!level_a)
                ky040_sub_angle_variation(encoder);
            else
                ky040_add_angle_variation(encoder);
        }
    } 
}

/*
    Manages interrupt caused by falling or rising edge interrupt of input pin
    Disables the interrupt of both pins to avoid debounce, handled by a timer
*/
static void IRAM_ATTR ky040_a_trigger_isr(void* arg) 
{
    KY040_t * encoder = (KY040_t *)arg;
    uint64_t count;
    if(!debounce_timer_activated)
    {
        debounce_timer_activated = true;    
        ky040_check_angle_variation(encoder, encoder->_pin_a);
        ESP_ERROR_CHECK(gptimer_start(debounce_timer));   
    }
    return;
}

/*
    Manages interrupt caused by falling or rising edge interrupt of input pin
    Disables the interrupt of both pins to avoid debounce, handled by a timer
*/
static void IRAM_ATTR ky040_b_trigger_isr(void* arg) 
{
    KY040_t * encoder = (KY040_t *)arg;
    uint64_t count;
    if(!debounce_timer_activated)
    {
        debounce_timer_activated = true;    
        ky040_check_angle_variation(encoder, encoder->_pin_b);    
        ESP_ERROR_CHECK(gptimer_start(debounce_timer));
    }
    return;
}

/*
    Re-enables input pins interrupt and restores debounce timer to 0
*/
static void IRAM_ATTR ky040_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg)
{    
    ESP_ERROR_CHECK(gptimer_stop(timer));
    ESP_ERROR_CHECK(gptimer_set_raw_count(timer, 0ULL));
    debounce_timer_activated = false;
    return;
}

/* Public functions & routines */
void ky040_init(KY040_t * encoder, uint8_t pin_a, uint8_t pin_b, uint8_t angle_increment)
{
    // GPIO & interrupt settings
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL<<pin_a) | (1ULL<<pin_b),
        .pull_up_en = 1
    };             
	ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_a, ky040_a_trigger_isr, (KY040_t *) encoder));
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_b, ky040_b_trigger_isr, (KY040_t *) encoder));
    ESP_LOGI(TAG, "GPIO config completed successfully");

    // Debounce timer settings
    gptimer_config_t timer_config = {
        .intr_priority = 0,
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &debounce_timer));
    gptimer_alarm_config_t timer_alarm_config = {
        .flags.auto_reload_on_alarm = 0,
        .alarm_count = 1000
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(debounce_timer, &timer_alarm_config));
    ESP_ERROR_CHECK(gptimer_set_raw_count(debounce_timer, 0ULL));
    gptimer_event_callbacks_t timer_event_callbacks = {
        .on_alarm = ky040_timer_isr
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(debounce_timer, &timer_event_callbacks, (KY040_t *) encoder));
    ESP_ERROR_CHECK(gptimer_enable(debounce_timer));
    ESP_LOGI(TAG, "Debounce timer setup completed successfully");

    encoder->_pin_a = pin_a;
    encoder->_pin_b = pin_b;

    encoder->_angle_increment = angle_increment;
    return;
}

void ky040_reset(KY040_t * encoder)
{
    encoder->_angle_variation = 0;
    return;
}

int ky040_get_angle_variation(KY040_t * encoder)
{
    return encoder->_angle_variation;
}