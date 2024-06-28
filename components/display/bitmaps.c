#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Components */

/* Private variables & defines */
#define TAG "Bitmaps"

/* Public functions & routines */

/*
    Convert bitmaps from horizontal mode to page mode
*/
void bitmap_horizontal_to_page_mode(uint8_t *horizontal_mode_bitmap, uint8_t *page_mode_bitmap, uint8_t width, uint8_t height)
{
    //height & width must be multiple of 8
    if((width % 8) != 0 || (height % 8) != 0)
    {
        ESP_LOGE(TAG, "Width or height not multiple of 8");
        return;
    }

    uint8_t rows = height/8;
    uint8_t cols = width/8;

    for(int z=0; z<rows; z++)
    {
        for(int k=0; k<cols; k++)
        {
            for(int j=0; j<8; j++)
            {
                page_mode_bitmap[j + k*8 + z*width] = 0;
                for(int i=0; i<8; i++)
                {
                    page_mode_bitmap[j + k*8 + z*width] |= ((horizontal_mode_bitmap[k + i*cols + z*width] & (0x80 >> j)) << j >> i);
                }
            }
        }
    }
    return;
}

/*
    Bitwise OR operation between 2 bitmaps
    Page_mode bitmaps ONLY!
*/
void bitmap_sum(uint8_t * bitmap_a, uint8_t * bitmap_b, uint8_t width_a, uint8_t height_a, uint8_t width_b, uint8_t height_b, uint8_t x_pos, uint8_t y_pos)
{
    //bitmap_a height & width must be multiple of 8
    if((width_a % 8) != 0 || (height_a % 8) != 0)
    {
        ESP_LOGE(TAG, "Width or height not multiple of 8");
        return;
    }

    //bitmap_b must fit bitmap_a dimensions
    if((width_b + x_pos) > width_a || (height_b + y_pos) > height_a)
    {
        ESP_LOGE(TAG, "Second bitmap cannot fit first bitmap dimensions");
        return;
    }

    //x_pos & y_pos starting from bottom-left corner
    for(uint8_t page=0; page<height_a/8; page++)
    {
        for(uint8_t seg=x_pos; seg<x_pos+width_b; seg++)
        {
            bitmap_a[seg + (width_a * (((height_a/8) - 1) - page))] |= bitmap_b[seg - x_pos + (width_b * (((height_b/8) - 1) - page))];
        }
    }
    return;
}