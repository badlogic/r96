#include <MiniFB.h>
#include <math.h>
#include <stdio.h>
#include "MiniFB_enums.h"
#include "r96/r96.h"

void line_naive(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = (x2 - x1);
	int32_t delta_y = (y2 - y1);
	int32_t num_pixels_x = abs(delta_x);
	int32_t num_pixels_y = abs(delta_y);

	if (delta_x == 0) {
		r96_vline(image, x1, y1, y2, color);
		return;
	}
	if (delta_y == 0) {
		r96_hline(image, x1, x2, y1, color);
		return;
	}

	float step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = delta_x < 0 ? -1 : 1;
		step_y = (float) delta_y / num_pixels_x;
		num_pixels = num_pixels_x;
	} else {
		step_x = (float) delta_x / num_pixels_y;
		step_y = delta_y < 0 ? -1 : 1;
		num_pixels = num_pixels_y;
	}

	float x = x1 + 0.5f, y = y1 + 0.5f;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_set_pixel(image, (int32_t) x, (int32_t) y, color);
		x += step_x;
		y += step_y;
	}
}

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("16_naive_line", window_width * 2, window_height * 2);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));
		int32_t mouse_x = mfb_get_mouse_x(window);
		int32_t mouse_y = mfb_get_mouse_y(window);
		line_naive(&output, 160, 100, mouse_x, mouse_y, 0xffff0000);

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
