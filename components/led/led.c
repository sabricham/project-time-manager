#include "led.h"

//======================================================================================
/* 
*   Macros
*/
//======================================================================================

#define TAG             "Led"

//======================================================================================
/* 
*   Private variables & defines
*/
//======================================================================================

QueueHandle_t ledQueue = NULL;
queueMessage ledQueueMessage;

typedef enum{
    ledEffectNone = 0,
    ledEffectRainbow,
    ledEffectBreath,
    ledEffectSolid,
    ledEffectLoading
} ledEffect_e;

led_strip_handle_t ledStripWS2812;
uint8_t ledEffectActive = 0;
uint8_t red, green, blue;
uint16_t ledEffectDelay = 1;

float ledEffectRainbowHueShift = 0;
float ledEffectBreathDiffuser = 1;
uint8_t ledEffectBreathReverse = 0;
int ledEffectPercentage = 0;

//======================================================================================
/* 
*   Private functions & routines
*/
//======================================================================================
/*
*   Reset all led variables before selecting another led effect
*/
void ResetLedVariables()
{    
    ledEffectRainbowHueShift = 0;
    ledEffectBreathDiffuser = 1;
    ledEffectBreathReverse = 0;
    ledEffectPercentage = 0;
}

/*
*   Clamp a value between a max and min, if above max returns max, if below min returns min
*/
float ClampFloat(float value, float max, float min)
{
    if(value > max) return max;
    if(value < min) return min;
    return value;
}

/*
*   Initialize a led strip with its settings using an RMT Backend
*/
led_strip_handle_t LedStripInitRMTBackend(led_model_t ledModel)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t stripConfig = {
        .strip_gpio_num = LED_STRIP_WS2812B_DATA_PIN, // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_WS2812B_NUM_LEDS,      // The number of LEDs in the strip,
        .led_model = ledModel,        // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color order of the strip: GRB
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmtConfig = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .mem_block_symbols = 64,               // the memory size of each RMT channel, in words (4 bytes)
        .flags = {
            .with_dma = false, // DMA feature is available on chips like ESP32-S3/P4
        }
    };

    // LED Strip object handle
    led_strip_handle_t ledStrip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &ledStrip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    return ledStrip;
}

/*
*   Initialize a led strip with its settings using an SPI Backend
*/
led_strip_handle_t LedStripInitSPIBackend(led_model_t ledModel)
{
    /// LED strip common configuration
    led_strip_config_t stripConfig = 
    {
        .strip_gpio_num = LED_STRIP_WS2812B_DATA_PIN,  // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_WS2812B_NUM_LEDS,  // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812, // LED strip model, it determines the bit timing
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color component format is G-R-B
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    /// SPI backend specific configuration
    led_strip_spi_config_t spiConfig = 
    {
        .clk_src = SPI_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
        .spi_bus = SPI2_HOST,           // SPI bus ID
        .flags = {
            .with_dma = true, // Using DMA can improve performance and help drive more LEDs
        }
    };

    /// Create the LED strip object    
    led_strip_handle_t ledStrip;
    ESP_ERROR_CHECK(led_strip_new_spi_device(&stripConfig, &spiConfig, &ledStrip));
    ESP_LOGI(TAG, "Created LED strip object with SPI backend");
    return ledStrip;
}

/*
*   Clear all leds by turning them off
*/
void ClearLed(led_strip_handle_t ledStrip)
{
    ESP_ERROR_CHECK(led_strip_clear(ledStrip));
}

/*
*   Set a color for a single led in the led strip, to show the changes use UpdateLedStrip
*/
void SetLedColor(led_strip_handle_t ledStrip, uint32_t ledIndex, uint8_t R, uint8_t G, uint8_t B)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(ledStrip, ledIndex, R, G, B));
}

/*
*   Update the led strip colors based on the SetLedColor done previously
*/
void UpdateLedStrip(led_strip_handle_t ledStrip)
{
    ESP_ERROR_CHECK(led_strip_refresh(ledStrip));
}

/*
*   Function to convert HSV to RGB
*/
void HSVtoRGB(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float c = v * s; // Chroma
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;
    float r_prime, g_prime, b_prime;

    if (h >= 0 && h < 60) {
        r_prime = c;
        g_prime = x;
        b_prime = 0;
    } else if (h >= 60 && h < 120) {
        r_prime = x;
        g_prime = c;
        b_prime = 0;
    } else if (h >= 120 && h < 180) {
        r_prime = 0;
        g_prime = c;
        b_prime = x;
    } else if (h >= 180 && h < 240) {
        r_prime = 0;
        g_prime = x;
        b_prime = c;
    } else if (h >= 240 && h < 300) {
        r_prime = x;
        g_prime = 0;
        b_prime = c;
    } else {
        r_prime = c;
        g_prime = 0;
        b_prime = x;
    }

    *r = (uint8_t)((r_prime + m) * 255);
    *g = (uint8_t)((g_prime + m) * 255);
    *b = (uint8_t)((b_prime + m) * 255);
}

/*
*   Visualize rainbow effect on the led strip
*/
void SetLedEffectRainbow(led_strip_handle_t ledStrip)
{
    uint8_t R, G, B;

    for (int i = 0; i < LED_STRIP_WS2812B_NUM_LEDS; i++) 
    {
        float hue = (360.0 / (LED_STRIP_WS2812B_NUM_LEDS)) * i + ledEffectRainbowHueShift;  // calculate hue for each LED
        if (hue > 360.0) hue -= 360.0;
        HSVtoRGB(hue, 1.0, 1.0, &R, &G, &B);
        SetLedColor(ledStrip, i, R, G, B);
    }
    UpdateLedStrip(ledStrip);
    
    // update hue shifter to make a full rotation
    ledEffectRainbowHueShift += 1;
    if (ledEffectRainbowHueShift > 360.0) ledEffectRainbowHueShift -= 360.0;
}

/*
*   Visualize color breathing effect on the led strip
*/
void SetLedEffectBreath(led_strip_handle_t ledStrip)
{
    float multiplier = pow(ClampFloat(ledEffectBreathDiffuser, 1.0, 0.0), 2);   // cubic function

    for (int i = 0; i < LED_STRIP_WS2812B_NUM_LEDS; i++) 
    {
        SetLedColor(ledStrip, i, red * multiplier, green * multiplier, blue * multiplier);
    }
    UpdateLedStrip(ledStrip);

    // update multiplier for breathing with linear function
    if (ledEffectBreathDiffuser >= 1)
    {
        ledEffectBreathReverse = 1;
    }
    else if(ledEffectBreathDiffuser <= 0)
    {
        ledEffectBreathReverse = 0;
    }

    if(ledEffectBreathReverse == 0)
    {        
        ledEffectBreathDiffuser += 0.01;
    }
    else
    {
        ledEffectBreathDiffuser -= 0.01;
    }
}

/*
*   Visualize solid color effect on the led strip
*/
void SetLedEffectSolid(led_strip_handle_t ledStrip)
{
    for (int i = 0; i < LED_STRIP_WS2812B_NUM_LEDS; i++) 
    {
        SetLedColor(ledStrip, i, red, green, blue);
    }
    UpdateLedStrip(ledStrip);
}

/*
*   Visualize loading color effect on the led strip
*/
void SetLedEffectLoading(led_strip_handle_t ledStrip)
{
    ESP_LOGI(TAG, "Percentage %d", ledEffectPercentage);
    int numLedsFull = (ledEffectPercentage * LED_STRIP_WS2812B_NUM_LEDS) / 10000;

    for (int i = 0; i < LED_STRIP_WS2812B_NUM_LEDS; i++) 
    {
        if(i < numLedsFull)
        {
            SetLedColor(ledStrip, LED_STRIP_WS2812B_NUM_LEDS-i-1, red, green, blue);
        }
        else if (i == numLedsFull)
        {
            int multiplier = ((ledEffectPercentage * LED_STRIP_WS2812B_NUM_LEDS) / 100) % 100;
            SetLedColor(ledStrip, LED_STRIP_WS2812B_NUM_LEDS-i-1, (red * multiplier)/100, (green * multiplier)/100, (blue * multiplier)/100);
        }        
        else
        {
            SetLedColor(ledStrip, LED_STRIP_WS2812B_NUM_LEDS-i-1, 0, 0, 0);
        }
    }
    UpdateLedStrip(ledStrip);
}

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
*   Entry point of the task
*/
void LedTask()
{
    //======================================================================================
    ESP_LOGW(TAG, "Starting task");

    esp_task_wdt_add(NULL);
    CreateQueue(&ledQueue, &ledQueueMessage, 3, TAG);

    ledStripWS2812 = LedStripInitRMTBackend(LED_MODEL_WS2812);

    vTaskDelay(pdMS_TO_TICKS(LED_TASK_STARTUP_DELAY));

    ESP_LOGW(TAG, "Task started correctly");
    //======================================================================================

    while(1)
    {
        // Get messages from other tasks
        if(xQueueReceive(ledQueue, &ledQueueMessage, 0))
        {
            ESP_LOGI(TAG, "Received message");
            if(ledQueueMessage.senderID == SENDER_ID_MANAGER)
            {
                switch(ledQueueMessage.messageID)
                {
                    case MESSAGE_ID_LED_CLEAR_LED:
                    {
                        ESP_LOGI(TAG, "Clear led strip");
                        ResetLedVariables();
                        ledEffectActive = ledEffectNone;
                        
                        ledEffectDelay = (uint16_t)ledQueueMessage.params[0];
                    }
                    break;
                    case MESSAGE_ID_LED_SET_EFFECT_RAINBOW:
                    {
                        ESP_LOGI(TAG, "Set rainbow effect on led strip");
                        ResetLedVariables();
                        ledEffectActive = ledEffectRainbow;
                        
                        ledEffectDelay = (uint16_t)ledQueueMessage.params[0];
                    }
                    break;
                    case MESSAGE_ID_LED_SET_EFFECT_BREATH:
                    {
                        ESP_LOGI(TAG, "Set breath effect on led strip");
                        ResetLedVariables();
                        ledEffectActive = ledEffectBreath;
                        
                        ledEffectDelay = (uint16_t)ledQueueMessage.params[0];

                        red = (uint8_t)ledQueueMessage.params[1];
                        green = (uint8_t)ledQueueMessage.params[2];
                        blue = (uint8_t)ledQueueMessage.params[3];
                    }
                    break;
                    case MESSAGE_ID_LED_SET_EFFECT_SOLID:
                    {
                        ESP_LOGI(TAG, "Set solid color on led strip");
                        ResetLedVariables();
                        ledEffectActive = ledEffectSolid;
                        
                        ledEffectDelay = (uint16_t)ledQueueMessage.params[0];

                        red = (uint8_t)ledQueueMessage.params[1];
                        green = (uint8_t)ledQueueMessage.params[2];
                        blue = (uint8_t)ledQueueMessage.params[3];               
                    }
                    break;                    
                    case MESSAGE_ID_LED_SET_EFFECT_LOADING:
                    {
                        ESP_LOGI(TAG, "Set loading effect on led strip"); 
                        ResetLedVariables();        
                        ledEffectActive = ledEffectLoading;
                        
                        ledEffectDelay = (uint16_t)ledQueueMessage.params[0];
                        
                        red = (uint8_t)ledQueueMessage.params[1];
                        green = (uint8_t)ledQueueMessage.params[2];
                        blue = (uint8_t)ledQueueMessage.params[3];

                        ledEffectPercentage = ledQueueMessage.params[4];
                    }
                    break;
                }
            }
        }

        // Task loop
        switch(ledEffectActive)
        {
            case ledEffectNone:
            {
                ClearLed(ledStripWS2812);
            }
            break;
            case ledEffectRainbow:
            {
                SetLedEffectRainbow(ledStripWS2812);
            }
            break;
            case ledEffectBreath:
            {
                SetLedEffectBreath(ledStripWS2812);
            }
            break;
            case ledEffectSolid:
            {
                SetLedEffectSolid(ledStripWS2812);
            }
            break;            
            case ledEffectLoading:
            {
                SetLedEffectLoading(ledStripWS2812);
            }
            break;
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(ledEffectDelay));
    }
    
    ESP_LOGE(TAG, "This section should not be reached");
    return;
}