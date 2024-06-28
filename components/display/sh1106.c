#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/* Components */
#include "sh1106.h"

/* Private variables & defines */
#define TAG "SH1106"
#define PACK8 __attribute__((aligned( __alignof__( uint8_t ) ), packed ))

typedef union out_column_t {
	uint32_t u32;
	uint8_t  u8[4];
} PACK8 out_column_t;

void sh1106_init(SH1106_t * dev, int width, int height)
{
    i2c_init(dev, width, height);
	// Initialize internal buffer
	for (int i=0;i<dev->_pages;i++) {
		memset(dev->_page[i]._segs, 0, 132);
	}
}

int sh1106_get_width(SH1106_t * dev)
{
	return dev->_width;
}

int sh1106_get_height(SH1106_t * dev)
{
	return dev->_height;
}

int sh1106_get_pages(SH1106_t * dev)
{
	return dev->_pages;
}

void sh1106_show_buffer(SH1106_t * dev)
{
	for (int page=0; page<dev->_pages;page++) {
        i2c_display_image(dev, page, 0, dev->_page[page]._segs, dev->_width);
    }
}

void sh1106_set_buffer(SH1106_t * dev, uint8_t * buffer)
{
	int index = 0;
	for (int page=0; page<dev->_pages;page++) {
		memcpy(&dev->_page[page]._segs, &buffer[index], 128);
		index = index + 128;
	}
}

void sh1106_get_buffer(SH1106_t * dev, uint8_t * buffer)
{
	int index = 0;
	for (int page=0; page<dev->_pages;page++) {
		memcpy(&buffer[index], &dev->_page[page]._segs, 128);
		index = index + 128;
	}
}

void sh1106_display_bitmap(SH1106_t * dev, uint8_t * bitmap, uint16_t bitmap_offset_x, uint16_t bitmap_width)
{
    for (uint8_t page=0;page<8;page++) 
    {
        uint8_t segs[130];
        memset(segs, 0, 130);
        for (uint8_t seg=0;seg<128;seg++) 
        {
            segs[seg+2] = sh1106_rotate_byte(bitmap[seg + page*bitmap_width + bitmap_offset_x]);
        }
            
        sh1106_display_image(dev, page, 0, segs, 130);
    }
}

void sh1106_display_image(SH1106_t * dev, int page, int seg, uint8_t * images, int width)
{
    i2c_display_image(dev, page, seg, images, width);

	// Set to internal buffer
	memcpy(&dev->_page[page]._segs[seg], images, width);
}

void sh1106_clear_screen(SH1106_t * dev, bool invert)
{
	uint8_t clear[1024];
	memset(clear, 0x00, sizeof(clear));
    sh1106_display_bitmap(dev, clear, 0, 128);
}

void sh1106_contrast(SH1106_t * dev, int contrast)
{
    i2c_contrast(dev, contrast);
}

// delay = 0 : display with no wait
// delay > 0 : display with wait
// delay < 0 : no display
void sh1106_wrap_arround(SH1106_t * dev, sh1106_scroll_type_t scroll, int start, int end, int8_t delay)
{
	if (scroll == SCROLL_RIGHT) {
		int _start = start; // 0 to 7
		int _end = end; // 0 to 7
		if (_end >= dev->_pages) _end = dev->_pages - 1;
		uint8_t wk;
		//for (int page=0;page<dev->_pages;page++) {
		for (int page=_start;page<=_end;page++) {
			wk = dev->_page[page]._segs[127];
			for (int seg=127;seg>0;seg--) {
				dev->_page[page]._segs[seg] = dev->_page[page]._segs[seg-1];
			}
			dev->_page[page]._segs[0] = wk;
		}

	} else if (scroll == SCROLL_LEFT) {
		int _start = start; // 0 to 7
		int _end = end; // 0 to 7
		if (_end >= dev->_pages) _end = dev->_pages - 1;
		uint8_t wk;
		//for (int page=0;page<dev->_pages;page++) {
		for (int page=_start;page<=_end;page++) {
			wk = dev->_page[page]._segs[0];
			for (int seg=0;seg<127;seg++) {
				dev->_page[page]._segs[seg] = dev->_page[page]._segs[seg+1];
			}
			dev->_page[page]._segs[127] = wk;
		}

	} else if (scroll == SCROLL_UP) {
		int _start = start; // 0 to {width-1}
		int _end = end; // 0 to {width-1}
		if (_end >= dev->_width) _end = dev->_width - 1;
		uint8_t wk0;
		uint8_t wk1;
		uint8_t wk2;
		uint8_t save[128];
		// Save pages 0
		for (int seg=0;seg<128;seg++) {
			save[seg] = dev->_page[0]._segs[seg];
		}
		// Page0 to Page6
		for (int page=0;page<dev->_pages-1;page++) {
			//for (int seg=0;seg<128;seg++) {
			for (int seg=_start;seg<=_end;seg++) {
				wk0 = dev->_page[page]._segs[seg];
				wk1 = dev->_page[page+1]._segs[seg];
				if (dev->_flip) wk0 = sh1106_rotate_byte(wk0);
				if (dev->_flip) wk1 = sh1106_rotate_byte(wk1);
				if (seg == 0) {
					ESP_LOGD(TAG, "b page=%d wk0=%02x wk1=%02x", page, wk0, wk1);
				}
				wk0 = wk0 >> 1;
				wk1 = wk1 & 0x01;
				wk1 = wk1 << 7;
				wk2 = wk0 | wk1;
				if (seg == 0) {
					ESP_LOGD(TAG, "a page=%d wk0=%02x wk1=%02x wk2=%02x", page, wk0, wk1, wk2);
				}
				if (dev->_flip) wk2 = sh1106_rotate_byte(wk2);
				dev->_page[page]._segs[seg] = wk2;
			}
		}
		// Page7
		int pages = dev->_pages-1;
		//for (int seg=0;seg<128;seg++) {
		for (int seg=_start;seg<=_end;seg++) {
			wk0 = dev->_page[pages]._segs[seg];
			wk1 = save[seg];
			if (dev->_flip) wk0 = sh1106_rotate_byte(wk0);
			if (dev->_flip) wk1 = sh1106_rotate_byte(wk1);
			wk0 = wk0 >> 1;
			wk1 = wk1 & 0x01;
			wk1 = wk1 << 7;
			wk2 = wk0 | wk1;
			if (dev->_flip) wk2 = sh1106_rotate_byte(wk2);
			dev->_page[pages]._segs[seg] = wk2;
		}

	} else if (scroll == SCROLL_DOWN) {
		int _start = start; // 0 to {width-1}
		int _end = end; // 0 to {width-1}
		if (_end >= dev->_width) _end = dev->_width - 1;
		uint8_t wk0;
		uint8_t wk1;
		uint8_t wk2;
		uint8_t save[128];
		// Save pages 7
		int pages = dev->_pages-1;
		for (int seg=0;seg<128;seg++) {
			save[seg] = dev->_page[pages]._segs[seg];
		}
		// Page7 to Page1
		for (int page=pages;page>0;page--) {
			//for (int seg=0;seg<128;seg++) {
			for (int seg=_start;seg<=_end;seg++) {
				wk0 = dev->_page[page]._segs[seg];
				wk1 = dev->_page[page-1]._segs[seg];
				if (dev->_flip) wk0 = sh1106_rotate_byte(wk0);
				if (dev->_flip) wk1 = sh1106_rotate_byte(wk1);
				if (seg == 0) {
					ESP_LOGD(TAG, "b page=%d wk0=%02x wk1=%02x", page, wk0, wk1);
				}
				wk0 = wk0 << 1;
				wk1 = wk1 & 0x80;
				wk1 = wk1 >> 7;
				wk2 = wk0 | wk1;
				if (seg == 0) {
					ESP_LOGD(TAG, "a page=%d wk0=%02x wk1=%02x wk2=%02x", page, wk0, wk1, wk2);
				}
				if (dev->_flip) wk2 = sh1106_rotate_byte(wk2);
				dev->_page[page]._segs[seg] = wk2;
			}
		}
		// Page0
		//for (int seg=0;seg<128;seg++) {
		for (int seg=_start;seg<=_end;seg++) {
			wk0 = dev->_page[0]._segs[seg];
			wk1 = save[seg];
			if (dev->_flip) wk0 = sh1106_rotate_byte(wk0);
			if (dev->_flip) wk1 = sh1106_rotate_byte(wk1);
			wk0 = wk0 << 1;
			wk1 = wk1 & 0x80;
			wk1 = wk1 >> 7;
			wk2 = wk0 | wk1;
			if (dev->_flip) wk2 = sh1106_rotate_byte(wk2);
			dev->_page[0]._segs[seg] = wk2;
		}

	}

	if (delay >= 0) {
		for (int page=0;page<dev->_pages;page++) {
            i2c_display_image(dev, page, 0, dev->_page[page]._segs, 128);
			if (delay) vTaskDelay(delay);
		}
	}

}

void sh1106_bitmaps(SH1106_t * dev, int xpos, int ypos, uint8_t * bitmap, int width, int height, bool invert)
{
	if ( (width % 8) != 0  || (height % 8) != 0) {
		ESP_LOGE(TAG, "width must be a multiple of 8");
		return;
	}
	int _width = width / 8;
	uint8_t wk0;
	uint8_t wk1;
	uint8_t wk2;
	uint8_t page = (ypos / 8);
	uint8_t _seg = xpos;
	uint8_t dstBits = (ypos % 8);
	ESP_LOGD(TAG, "ypos=%d page=%d dstBits=%d", ypos, page, dstBits);
	int offset = 0;
	for(int _height=0;_height<height;_height++) {
		for (int index=0;index<_width;index++) {
			for (int srcBits=7; srcBits>=0; srcBits--) {
				wk0 = dev->_page[page]._segs[_seg];
				if (dev->_flip) wk0 = sh1106_rotate_byte(wk0);

				wk1 = bitmap[index+offset];
				if (invert) wk1 = ~wk1;

				//wk2 = sh1106_copy_bit(bitmap[index+offset], srcBits, wk0, dstBits);
				wk2 = sh1106_copy_bit(wk1, srcBits, wk0, dstBits);
				if (dev->_flip) wk2 = sh1106_rotate_byte(wk2);

				ESP_LOGD(TAG, "index=%d offset=%d page=%d _seg=%d, wk2=%02x", index, offset, page, _seg, wk2);
				dev->_page[page]._segs[_seg] = wk2;
				_seg++;
			}
		}
		vTaskDelay(1);
		offset = offset + _width;
		dstBits++;
		_seg = xpos;
		if (dstBits == 8) {
			page++;
			dstBits=0;
		}
	}

#if 0
	for (int _seg=ypos;_seg<ypos+width;_seg++) {
		sh1106_dump_page(dev, page-1, _seg);
	}
	for (int _seg=ypos;_seg<ypos+width;_seg++) {
		sh1106_dump_page(dev, page, _seg);
	}
#endif
	sh1106_show_buffer(dev);
}


// Set pixel to internal buffer. Not show it.
void _sh1106_pixel(SH1106_t * dev, int xpos, int ypos, bool invert)
{
	uint8_t _page = (ypos / 8);
	uint8_t _bits = (ypos % 8);
	uint8_t _seg = xpos;
	uint8_t wk0 = dev->_page[_page]._segs[_seg];
	uint8_t wk1 = 1 << _bits;
	ESP_LOGD(TAG, "ypos=%d _page=%d _bits=%d wk0=0x%02x wk1=0x%02x", ypos, _page, _bits, wk0, wk1);
	if (invert) {
		wk0 = wk0 & ~wk1;
	} else {
		wk0 = wk0 | wk1;
	}
	if (dev->_flip) wk0 = sh1106_rotate_byte(wk0);
	ESP_LOGD(TAG, "wk0=0x%02x wk1=0x%02x", wk0, wk1);
	dev->_page[_page]._segs[_seg] = wk0;
}

// Set line to internal buffer. Not show it.
void _sh1106_line(SH1106_t * dev, int x1, int y1, int x2, int y2,  bool invert)
{
	int i;
	int dx,dy;
	int sx,sy;
	int E;

	/* distance between two points */
	dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
	dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

	/* direction of two point */
	sx = ( x2 > x1 ) ? 1 : -1;
	sy = ( y2 > y1 ) ? 1 : -1;

	/* inclination < 1 */
	if ( dx > dy ) {
		E = -dx;
		for ( i = 0 ; i <= dx ; i++ ) {
			_sh1106_pixel(dev, x1, y1, invert);
			x1 += sx;
			E += 2 * dy;
			if ( E >= 0 ) {
			y1 += sy;
			E -= 2 * dx;
		}
	}

	/* inclination >= 1 */
	} else {
		E = -dy;
		for ( i = 0 ; i <= dy ; i++ ) {
			_sh1106_pixel(dev, x1, y1, invert);
			y1 += sy;
			E += 2 * dx;
			if ( E >= 0 ) {
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}

void sh1106_invert(uint8_t *buf, size_t blen)
{
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}

// Flip upside down
void sh1106_flip(uint8_t *buf, size_t blen)
{
	for(int i=0; i<blen; i++){
		buf[i] = sh1106_rotate_byte(buf[i]);
	}
}

uint8_t sh1106_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits)
{
	ESP_LOGD(TAG, "src=%02x srcBits=%d dst=%02x dstBits=%d", src, srcBits, dst, dstBits);
	uint8_t smask = 0x01 << srcBits;
	uint8_t dmask = 0x01 << dstBits;
	uint8_t _src = src & smask;
#if 0
	if (_src != 0) _src = 1;
	uint8_t _wk = _src << dstBits;
	uint8_t _dst = dst | _wk;
#endif
	uint8_t _dst;
	if (_src != 0) {
		_dst = dst | dmask; // set bit
	} else {
		_dst = dst & ~(dmask); // clear bit
	}
	return _dst;
}


// Rotate 8-bit data
// 0x12-->0x48
uint8_t sh1106_rotate_byte(uint8_t ch1) {
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}


void sh1106_fadeout(SH1106_t * dev)
{
	void (*func)(SH1106_t * dev, int page, int seg, uint8_t * images, int width);
    func = i2c_display_image;

	uint8_t image[1];
	for(int page=0; page<dev->_pages; page++) {
		image[0] = 0xFF;
		for(int line=0; line<8; line++) {
			if (dev->_flip) {
				image[0] = image[0] >> 1;
			} else {
				image[0] = image[0] << 1;
			}
			for(int seg=0; seg<128; seg++) {
				(*func)(dev, page, seg, image, 1);
				dev->_page[page]._segs[seg] = image[0];
			}
		}
	}
}

void sh1106_dump(SH1106_t dev)
{
	printf("_address=%x\n",dev._address);
	printf("_width=%x\n",dev._width);
	printf("_height=%x\n",dev._height);
	printf("_pages=%x\n",dev._pages);
}

void sh1106_dump_page(SH1106_t * dev, int page, int seg)
{
	ESP_LOGI(TAG, "dev->_page[%d]._segs[%d]=%02x", page, seg, dev->_page[page]._segs[seg]);
}

