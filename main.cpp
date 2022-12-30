/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"




#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <array>
#include <vector>

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define ILI9341_RESET 21
#define ILI9341_DC 20
#define ILI9341_SCK 18
#define ILI9341_MOSI 19
#define ILI9341_MISO 16
#define ILI9341_CS 17

#define ILI9341_SPI_PORT spi0

class ILI9341LCD {
public:
	static constexpr int32_t WIDTH = 320;
	static constexpr int32_t HEIGHT = 240;
private:
	void initialize_io()
	{
		spi_init(ILI9341_SPI_PORT, 50 * 1000 * 1000);
		spi_set_format(ILI9341_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
		gpio_set_function(ILI9341_SCK, GPIO_FUNC_SPI);
		gpio_set_function(ILI9341_MOSI, GPIO_FUNC_SPI);
		gpio_set_function(ILI9341_MISO, GPIO_FUNC_SPI);

		gpio_init(ILI9341_CS);
		gpio_set_dir(ILI9341_CS, GPIO_OUT);
		cs(1);

		gpio_init(ILI9341_DC);
		gpio_set_dir(ILI9341_DC, GPIO_OUT);

		gpio_init(ILI9341_RESET);
		gpio_set_dir(ILI9341_RESET, GPIO_OUT);

		reset();
	}
	void initialize_device(void)
	{
		write_command(0x01);
		sleep_ms(50);
		write_command(0x11);
		sleep_ms(50);

		write_command(0xb6);
		write_data16(0x0ac2);
		write_command(0x36);
		write_data8(0x68);
		write_command(0x3a);
		write_data8(0x55);

		write_command(0x29);
	}
	void reset()
	{
		gpio_put(ILI9341_RESET, 1);
		sleep_ms(1);
		gpio_put(ILI9341_RESET, 0);
		sleep_ms(10);
		gpio_put(ILI9341_RESET, 1);
		sleep_ms(120);
	}
	void dc(bool f)
	{
		gpio_put(ILI9341_DC, f);
	}
	void cs(bool f)
	{
		gpio_put(ILI9341_CS, f);
	}
	void write_command(uint8_t cmd)
	{
		dc(0);
		cs(0);
		spi_write_blocking(ILI9341_SPI_PORT, &cmd, 1);
		cs(1);
	}
	void write_data8(uint8_t data)
	{
		dc(1);
		cs(0);
		spi_write_blocking(ILI9341_SPI_PORT, &data, 1);
		cs(1);
	}
	void write_data16(uint16_t data)
	{
		uint8_t tmp[2];
		tmp[0] = data >> 8;
		tmp[1] = data;
		dc(1);
		cs(0);
		spi_write_blocking(ILI9341_SPI_PORT, tmp, 2);
		cs(1);
	}
	void write_data(const uint8_t *ptr, int32_t len)
	{
		dc(1);
		cs(0);
		spi_write_blocking(ILI9341_SPI_PORT, ptr, len);
		cs(1);
	}
	void read_data(uint8_t cmd, uint8_t *ptr, int32_t len)
	{
		dc(0);
		cs(0);
		spi_write_blocking(ILI9341_SPI_PORT, &cmd, 1);

		gpio_put(ILI9341_DC, 1);
		spi_read_blocking(ILI9341_SPI_PORT, 0, ptr, len);
		cs(1);
	}
	void set_area(int32_t x, int32_t y, int32_t w, int32_t h)
	{
		write_command(0x2a);
		write_data16(x);
		write_data16(x + w - 1);
		write_command(0x2b);
		write_data16(y);
		write_data16(y + h - 1);
	}
public:
	ILI9341LCD() {}
	~ILI9341LCD() {}
	void initialize()
	{
		initialize_io();
		initialize_device();
		set_area(0, 0, WIDTH, HEIGHT);
	}
	void finalize()
	{
		spi_deinit(ILI9341_SPI_PORT);
	}
	static uint16_t color(uint8_t r, uint8_t g, uint8_t b)
	{
		return ((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | ((b >> 3) & 0x001f);
	}
	void clear(uint16_t color)
	{
		fill_rect(0, 0, WIDTH, HEIGHT, color);
	}
	void draw_pixel(int32_t x, int32_t y, uint16_t color)
	{
		set_area(x, y, 1, 1);
		write_command(0x2c);
		write_data16(color);
	}
	void fill_rect(int x, int y, int w, int h, uint16_t color)
	{
		set_area(x, y, w, h);
		write_command(0x2c);
		for (int32_t i = 0; i < w * h; i++) {
			write_data16(color);
		}
	}
	void draw_buffer(int x, int y, int w, int h, uint16_t const *p)
	{
		set_area(x, y, w, h);
		write_command(0x2c);
		for (int i = 0; i < w * h; i++) {
			write_data16(p[i]);
		}
	}
	void draw_line(int x0, int y0, int x1, int y1, uint16_t color)
	{
		int dx = abs(x1 - x0);
		int sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0);
		int sy = y0 < y1 ? 1 : -1;
		int e = dx + dy;
		while (1) {
			draw_pixel(x0, y0, color);
			if (x0 == x1 && y0 == y1) break;
			int e2 = 2 * e;
			if (e2 >= dy) {
				e += dy;
				x0 += sx;
			}
			if (e2 <= dx) {
				e += dx;
				y0 += sy;
			}
		}
	}
	void test()
	{
		int x0 = rand() % 320;
		int y0 = rand() % 240;
		int x1 = rand() % 320;
		int y1 = rand() % 240;
		int r = rand() % 256;
		int g = rand() % 256;
		int b = rand() % 256;
		draw_line(x0, y0, x1, y1, color(r, g, b));
	}
};


ILI9341LCD *lcd;





uint8_t keyboard_data[8];

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 500 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED = 500,
	BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
	blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
	blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void)remote_wakeup_en;
	blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
	blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id)
{
	// skip if hid is not ready yet
	if (!tud_hid_ready()) return;

//	board_led_write(btn);

	switch (report_id) {
	case REPORT_ID_KEYBOARD:
		tud_hid_report(REPORT_ID_KEYBOARD, keyboard_data, 8);
		break;

	case REPORT_ID_MOUSE:
		{
//			int8_t const delta = 5;
			// no button, right + down, no scroll, no pan
//			tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
		}
		break;
	}
}

bool key_sw0 = 0;
uint8_t keys[5];

bool read_sw0()
{
	return !gpio_get(22);
}

int col_to_gpio(int col)
{
	return col + 10;
}

void deselect_cols()
{
	for (int col = 0; col < 5; col++) {
		int gpio = col_to_gpio(col);
		gpio_set_dir(gpio, GPIO_IN);
		gpio_pull_up(gpio);
	}
}

void select_col(int col)
{
	int gpio = col_to_gpio(col);
	gpio_set_dir(gpio, GPIO_OUT);
	gpio_put(gpio, false);
}

bool read_col(int col)
{
	int gpio = col_to_gpio(col);
	gpio_set_dir(gpio, GPIO_IN);
	return !gpio_get(gpio);
}

static const uint8_t keymap[][5] = {
	  0, 100, 101, 102, 103,
	104,   0, 105, 106, 107,
	108, 109,   0, 110, 111,
	112, 113, 114,   0, 115,
	116, 117, 118, 119,   0,
};

static const uint8_t keymap2[] = {
	HID_KEY_A, HID_KEY_B, HID_KEY_C, HID_KEY_D,
	HID_KEY_E, HID_KEY_F, HID_KEY_G, HID_KEY_H,
	HID_KEY_I, HID_KEY_J, HID_KEY_K, HID_KEY_L,
	HID_KEY_M, HID_KEY_N, HID_KEY_O, HID_KEY_P,
	HID_KEY_Q, HID_KEY_R, HID_KEY_S, HID_KEY_T,
};









void clear_key(uint8_t key)
{
	if (key > 0) {
		if (key >= 0xe0 && key < 0xe8) {
			keyboard_data[0] &= ~(1 << (key - 0xe0));
		}
		uint8_t i = 8;
		while (i > 2) {
			i--;
			if (keyboard_data[i] == key) {
				if (i < 7) {
					memmove(keyboard_data + i, keyboard_data + i + 1, 7 - i);
				}
				keyboard_data[7] = 0;
			}
		}
	}
}

void release_all_keys()
{
	memset(keyboard_data + 2, 0, 6);
}

void release_key(uint8_t key)
{
	clear_key(key);
	keyboard_data[1] = 0;
}

void press_key(uint8_t key)
{
	if (key >= 0xe0 && key < 0xe8) {
		keyboard_data[0] |= 1 << (key - 0xe0);
	} else if (key > 0) {
		clear_key(key);
		memmove(keyboard_data + 3, keyboard_data + 2, 5);
		keyboard_data[2] = key;
	}
	keyboard_data[1] = 0;
}

bool read_rows()
{
	bool changed = false;
	for (int r = 0; r < 5; r++) {
		uint8_t bits = 0;

		select_col(4 - r);
		sleep_us(1);
		for (int c = 0; c < 5; c++) {
			bits <<= 1;
			if (read_col(4 - c)) {
				bits |= 1;
			}
		}
		deselect_cols();

		uint8_t diff = keys[r] ^ bits;
		keys[r] = bits;

		if (diff != 0) {
			changed = true;
			for (int c = 0; c < 5; c++) {
				if ((diff >> c) & 1) {
					uint8_t key = keymap[r][4 - c];
					if (key >= 100 && key < 120) {
						key = keymap2[key - 100];
						bool press = (bits >> c) & 1;
						if (press) {
							press_key(key);
						} else {
							release_key(key);
						}
					}
				}
			}
		}
	}

	bool sw0 = read_sw0();
	if (key_sw0 != sw0) {
		changed = true;
		key_sw0 = sw0;
		uint8_t key = HID_KEY_ESCAPE;
		if (sw0) {
			press_key(key);
		} else {
			release_key(key);
		}
	}

	return changed;
}


// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
	// Poll every 10ms
	const uint32_t interval_ms = 10;
	static uint32_t start_ms = 0;

	if (board_millis() - start_ms < interval_ms) return; // not enough time
	start_ms += interval_ms;

	bool btn = read_rows();

	// Remote wakeup
	if (tud_suspended() && btn) {
		// Wake up host if we are in suspend mode
		// and REMOTE_WAKEUP feature is enabled by host
		tud_remote_wakeup();
	} else {
		// Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
		send_hid_report(REPORT_ID_KEYBOARD);
	}
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
	(void)instance;
	(void)len;

	uint8_t next_report_id = report[0] + 1;

	if (next_report_id < REPORT_ID_COUNT) {
		send_hid_report(next_report_id);
	}
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
	// TODO not Implemented
	(void)instance;
	(void)report_id;
	(void)report_type;
	(void)buffer;
	(void)reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
	(void)instance;

	if (report_type == HID_REPORT_TYPE_OUTPUT) {
		// Set keyboard LED e.g Capslock, Numlock etc...
		if (report_id == REPORT_ID_KEYBOARD) {
			// bufsize should be (at least) 1
			if (bufsize < 1) return;

			uint8_t const kbd_leds = buffer[0];
#if 0
			if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
				// Capslock On: disable blink, turn led on
				blink_interval_ms = 0;
				board_led_write(true);
			} else {
				// Caplocks Off: back to normal blink
				board_led_write(false);
				blink_interval_ms = BLINK_MOUNTED;
			}
#else
			board_led_write(kbd_leds & KEYBOARD_LED_CAPSLOCK);
#endif
		}
	}
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
	static uint32_t start_ms = 0;
	static bool led_state = false;

	// blink is disabled
	if (!blink_interval_ms) return;

	// Blink every interval ms
	if (board_millis() - start_ms < blink_interval_ms) return; // not enough time
	start_ms += blink_interval_ms;

#if 0
	board_led_write(led_state);
#endif
	led_state = 1 - led_state; // toggle
}

/*------------- MAIN -------------*/
int main(void)
{
	board_init();
	tusb_init();

	memset(keys, 0, sizeof(keys));
	memset(keyboard_data, 0, sizeof(keyboard_data));

	gpio_init(22);
	gpio_set_dir(22, GPIO_IN);
	gpio_pull_up(22);

	lcd = new ILI9341LCD();
	lcd->initialize();
	lcd->clear(ILI9341LCD::color(0, 0, 0));

	for (int i = 0; i < 5; i++) {
		int gpio = col_to_gpio(i);
		gpio_init(gpio);
	}

	deselect_cols();

	while (1) {
		tud_task(); // tinyusb device task
		led_blinking_task();

		hid_task();

		lcd->test();
	}

	return 0;
}

