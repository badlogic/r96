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

void line_bresenham(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int32_t delta_y = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int32_t error = delta_x + delta_y, error_2;

	for (;;) {
		r96_set_pixel(image, x1, y1, color);
		if (x1 == x2 && y1 == y2) break;
		error_2 = error << 1;
		if (error_2 >= delta_y) {
			error += delta_y;
			x1 += sx;
		}
		if (error_2 <= delta_x) {
			error += delta_x;
			y1 += sy;
		}
	}
}

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("18_line_perf", window_width * 2, window_height * 2);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/tamzen5x9.png", 5, 9);
	struct mfb_timer *timer = mfb_timer_create();
	do {
		srand(0);

		mfb_timer_reset(timer);
		for (int i = 0; i < 10000; i++) {
			line_naive(&output, rand() % 320, rand() % 240, rand() % 320, rand() % 240, rand() % 0xffffff + 0xff000000);
		}
		double naive_time = mfb_timer_delta(timer);

		mfb_timer_reset(timer);
		for (int i = 0; i < 10000; i++) {
			line_fixed_point(&output, rand() % 320, rand() % 240, rand() % 320, rand() % 240, rand() % 0xffffff + 0xff000000);
		}
		double fixed_point_time = mfb_timer_delta(timer);

		mfb_timer_reset(timer);
		for (int i = 0; i < 10000; i++) {
			line_bresenham(&output, rand() % 320, rand() % 240, rand() % 320, rand() % 240, rand() % 0xffffff + 0xff000000);
		}
		double bresenham_time = mfb_timer_delta(timer);

		char text[500];
		sprintf(text, "naive:       %f secs\nfixed point: %f secs\nbresenham:   %f secs", naive_time, fixed_point_time, bresenham_time);
		int32_t text_width, text_height;
		r96_font_get_text_bounds(&font, text, &text_width, &text_height);
		r96_rect(&output, 0, 0, text_width, text_height, 0xff222222);
		r96_text(&output, &font, text, 0, 0, 0xffffffff);

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
