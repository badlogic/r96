#include <MiniFB.h>
#include "r96/r96.h"

#define FIXED_8_BITS 8
#define FIXED_8_ZERO_POINT_FIVE (1 << (FIXED_8_BITS - 1))

static inline int32_t int_to_fixed(int32_t v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

static inline int32_t fixed_to_int(int32_t v, int32_t bits) {
	return v >> bits;
}

static inline int32_t fixed_div(int32_t a, int32_t b, int32_t bits) {
	return ((int64_t) a * (1 << bits)) / b;
}

void line_fixed_point(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = x2 - x1;
	int32_t delta_y = y2 - y1;
	int32_t num_pixels_x = abs(delta_x) + 1;
	int32_t num_pixels_y = abs(delta_y) + 1;
	delta_x = int_to_fixed(delta_x, FIXED_8_BITS);
	delta_y = int_to_fixed(delta_y, FIXED_8_BITS);

	int32_t step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = int_to_fixed(delta_x < 0 ? -1 : 1, FIXED_8_BITS);
		step_y = delta_x != 0 ? fixed_div(delta_y, abs(delta_x), FIXED_8_BITS) : 0;
		num_pixels = num_pixels_x;
	} else {
		step_x = delta_y != 0 ? fixed_div(delta_x, abs(delta_y), FIXED_8_BITS) : 0;
		step_y = int_to_fixed(delta_y < 0 ? -1 : 1, FIXED_8_BITS);
		num_pixels = num_pixels_y;
	}

	int32_t x = int_to_fixed(x1, FIXED_8_BITS) + FIXED_8_ZERO_POINT_FIVE, y = int_to_fixed(y1, FIXED_8_BITS) + FIXED_8_ZERO_POINT_FIVE;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_set_pixel(image, fixed_to_int(x, FIXED_8_BITS), fixed_to_int(y, FIXED_8_BITS), color);
		x += step_x;
		y += step_y;
	}
}

int main(void) {
	const int window_width = 40, window_height = 30;
	int scale = 16;
	struct mfb_window *window = mfb_open("17_fixed_point_line", window_width * scale, window_height * scale);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));
		int32_t mouse_x = mfb_get_mouse_x(window) / scale;
		int32_t mouse_y = mfb_get_mouse_y(window) / scale;
		line_fixed_point(&output, window_width / 2, window_height / 2, mouse_x, mouse_y, 0xffff0000);

		if (mfb_update_ex(window, output.pixels, output.width, output.height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
