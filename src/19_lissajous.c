#include <MiniFB.h>
#include <math.h>
#include "r96/r96.h"

#define FIXED_8_BITS 8
#define FIXED_8_ZERO_POINT_FIVE (1 << (FIXED_8_BITS - 1))

static inline int32_t float_to_fixed(float v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

static inline int32_t int_to_fixed(int32_t v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

static inline int32_t fixed_to_int(int32_t v, int32_t bits) {
	return v >> bits;
}

static inline int32_t fixed_div(int32_t a, int32_t b) {
	return ((int64_t) a * (1 << FIXED_8_BITS)) / b;
}

static inline int32_t fixed_round(int32_t v, int32_t bits) {
	int32_t fpOne = (1 << bits);
	int32_t subMask = fpOne - 1;
	return (v + subMask) & ~subMask;
}

void line_fixed_point(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
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

	int32_t step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = int_to_fixed(delta_x < 0 ? -1 : 1, FIXED_8_BITS);
		step_y = float_to_fixed((float) delta_y / num_pixels_x, FIXED_8_BITS);
		num_pixels = num_pixels_x;
	} else {
		step_x = float_to_fixed((float) delta_x / num_pixels_y, FIXED_8_BITS);
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

void line_sub_pixel(r96_image *image, float x1, float y1, float x2, float y2, float color) {
	int32_t x1_fp = float_to_fixed(x1, FIXED_8_BITS);
	int32_t y1_fp = float_to_fixed(y1, FIXED_8_BITS);
	int32_t x2_fp = float_to_fixed(x2, FIXED_8_BITS);
	int32_t y2_fp = float_to_fixed(y2, FIXED_8_BITS);
	int32_t delta_x = (x2_fp - x1_fp);
	int32_t delta_y = (y2_fp - y1_fp);
	int32_t num_pixels_x = abs(delta_x);
	int32_t num_pixels_y = abs(delta_y);

	if (delta_x == 0) {
		r96_vline(image, (int) x1, (int) y1, (int) y2, color);
		return;
	}
	if (delta_y == 0) {
		r96_hline(image, (int) x1, (int) x2, (int) y1, color);
		return;
	}

	int32_t step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = int_to_fixed(delta_x < 0 ? -1 : 1, FIXED_8_BITS);
		step_y = fixed_div(delta_y, num_pixels_x);
		num_pixels = fixed_to_int(fixed_round(num_pixels_x, FIXED_8_BITS), FIXED_8_BITS);
	} else {
		step_x = fixed_div(delta_x, num_pixels_y);
		step_y = int_to_fixed(delta_y < 0 ? -1 : 1, FIXED_8_BITS);
		num_pixels = fixed_to_int(fixed_round(num_pixels_y, FIXED_8_BITS), FIXED_8_BITS);
	}

	int32_t x = x1_fp, y = y1_fp;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_set_pixel(image, fixed_to_int(x, FIXED_8_BITS), fixed_to_int(y, FIXED_8_BITS), color);
		x += step_x;
		y += step_y;
	}
}

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("19_lissajous", window_width * 2, window_height * 2);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	struct mfb_timer *timer = mfb_timer_create();
	float delta = M_PI / 4, a = 4, b = 6;
	do {
		delta += mfb_timer_delta(timer) / 10;
		mfb_timer_reset(timer);

		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		float last_x = 0, last_y = 0;
		int segments = 100;
		for (float i = 0; i <= segments; i++) {
			float theta = i / (float) segments * M_PI;
			float x = 100 * sinf(a * theta + delta) / 2 + window_width / 2.0;
			float y = 100 * sinf(b * theta) / 2 + window_height / 2.0;
			if (i > 0) line_fixed_point(&output, (int) last_x - 75, (int) last_y, (int) x - 75, (int) y, 0xff00ff00);
			if (i > 0) line_sub_pixel(&output, last_x + 75, last_y, x + 75, y, 0xffff0000);
			last_x = x;
			last_y = y;
		}

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
