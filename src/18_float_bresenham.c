#include <MiniFB.h>
#include <math.h>
#include "r96/r96.h"

void line_float_bresenham(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	float delta_x = abs(x2 - x1);
	float delta_y = abs(y2 - y1);

	int32_t step_x = x1 < x2 ? 1 : -1;
	int32_t step_y = y1 < y2 ? 1 : -1;
	int32_t x = x1;
	int32_t y = y1;

	if (delta_x >= delta_y) {
		int32_t num_pixels = delta_x + 1;
		float slope = delta_x != 0 ? delta_y / delta_x : 0;
		float error = slope;
		for (int i = 0; i < num_pixels; i++) {
			r96_set_pixel(image, x, y, color);
			if (error > 0.5) {
				error -= 1;
				y += step_y;
			}
			error += slope;
			x += step_x;
		}
	} else {
		int32_t num_pixels = delta_y + 1;
		float slope = delta_y != 0 ? delta_x / delta_y : 0;
		float error = slope;
		for (int i = 0; i < num_pixels; i++) {
			r96_set_pixel(image, x, y, color);
			if (error > 0.5) {
				error -= 1;
				x += step_x;
			}
			error += slope;
			y += step_y;
		}
	}
}

int main(void) {
	const int window_width = 40, window_height = 30;
	int scale = 16;
	struct mfb_window *window = mfb_open("18_float_bresenham", window_width * scale, window_height * scale);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));
		int32_t mouse_x = mfb_get_mouse_x(window) / scale;
		int32_t mouse_y = mfb_get_mouse_y(window) / scale;
		line_float_bresenham(&output, window_width / 2, window_height / 2, mouse_x, mouse_y, 0xffff0000);

		if (mfb_update_ex(window, output.pixels, output.width, output.height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
