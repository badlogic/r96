#include <MiniFB.h>
#include <math.h>
#include <stdio.h>
#include "r96/r96.h"

void line_naive(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = x2 - x1;
	int32_t delta_y = y2 - y1;
	int32_t num_pixels_x = abs(delta_x) + 1;
	int32_t num_pixels_y = abs(delta_y) + 1;

	float step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = delta_x < 0 ? -1 : 1;
		step_y = delta_x != 0 ? (float) delta_y / abs(delta_x) : 0;
		num_pixels = num_pixels_x;
	} else {
		step_x = delta_y != 0 ? (float) delta_x / abs(delta_y) : 0;
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

void line_integer_bresenham(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = abs(x2 - x1);
	int32_t delta_y = abs(y2 - y1);
	int32_t delta_x_2 = delta_x << 1;
	int32_t delta_y_2 = delta_y << 1;

	int32_t step_x = x1 < x2 ? 1 : -1;
	int32_t step_y = y1 < y2 ? 1 : -1;
	int32_t x = x1;
	int32_t y = y1;

	if (delta_x >= delta_y) {
		int32_t num_pixels = delta_x + 1;
		int32_t error = delta_y_2 - delta_x;
		for (int i = 0; i < num_pixels; i++) {
			r96_set_pixel(image, x, y, color);
			if (error > 0) {
				error -= delta_x_2;
				y += step_y;
			}
			error += delta_y_2;
			x += step_x;
		}
	} else {
		int32_t num_pixels = delta_y + 1;
		int32_t error = delta_x_2 - delta_y;
		for (int i = 0; i < num_pixels; i++) {
			r96_set_pixel(image, x, y, color);
			if (error > 0) {
				error -= delta_y_2;
				x += step_x;
			}
			error += delta_x_2;
			y += step_y;
		}
	}
}

void line_optimized_bresenham(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = abs(x2 - x1);
	int32_t delta_y = abs(y2 - y1);
	int32_t major_delta, major_delta_2, major_step;
	int32_t minor_delta, minor_delta_2, minor_step;

	if (delta_x >= delta_y) {
		major_delta = delta_x;
		minor_delta = delta_y;
		major_step = x1 < x2 ? 1 : -1;
		minor_step = y1 < y2 ? image->width : -image->width;
	} else {
		major_delta = delta_y;
		minor_delta = delta_x;
		major_step = y1 < y2 ? image->width : -image->width;
		minor_step = x1 < x2 ? 1 : -1;
	}
	major_delta_2 = major_delta << 1;
	minor_delta_2 = minor_delta << 1;

	int32_t num_pixels = major_delta + 1;
	int32_t error = minor_delta_2 - major_delta;
	uint32_t *pixel = image->pixels + x1 + y1 * image->width;
	for (int i = 0; i < num_pixels; i++) {
		*pixel = color;
		if (error > 0) {
			error -= major_delta_2;
			pixel += minor_step;
		}
		error += minor_delta_2;
		pixel += major_step;
	}
}

int main(void) {
	const int window_width = 640, window_height = 480;
	struct mfb_window *window = mfb_open("21_line_perf", window_width, window_height);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/tamzen.png", 8, 16);
	struct mfb_timer *timer = mfb_timer_create();
	int iterations = 5000;
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < iterations; i++) {
			uint32_t color = MFB_ARGB(0xff, rand() & 0xff, rand() & 0xff, rand() & 0xff);
			line_naive(&output, rand() % window_width, rand() % window_height, rand() % window_width, rand() % window_height, color);
		}
		double naive_time = mfb_timer_delta(timer);

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < iterations; i++) {
			uint32_t color = MFB_ARGB(0xff, rand() & 0xff, rand() & 0xff, rand() & 0xff);
			line_fixed_point(&output, rand() % window_width, rand() % window_height, rand() % window_width, rand() % window_height, color);
		}
		double fixed_point_time = mfb_timer_delta(timer);

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < iterations; i++) {
			uint32_t color = MFB_ARGB(0xff, rand() & 0xff, rand() & 0xff, rand() & 0xff);
			line_float_bresenham(&output, rand() % window_width, rand() % window_height, rand() % window_width, rand() % window_height, color);
		}
		double float_bresenham_time = mfb_timer_delta(timer);

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < iterations; i++) {
			uint32_t color = MFB_ARGB(0xff, rand() & 0xff, rand() & 0xff, rand() & 0xff);
			line_integer_bresenham(&output, rand() % window_width, rand() % window_height, rand() % window_width, rand() % window_height, color);
		}
		double int_bresenham_time = mfb_timer_delta(timer);

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < iterations; i++) {
			uint32_t color = MFB_ARGB(0xff, rand() & 0xff, rand() & 0xff, rand() & 0xff);
			line_optimized_bresenham(&output, rand() % window_width, rand() % window_height, rand() % window_width, rand() % window_height, color);
		}
		double opt_bresenham_time = mfb_timer_delta(timer);

		char text[300];
		snprintf(text, 300, "DDA float:       %f secs\nDDA fixed point: %f secs\nBresenham float: %f secs\nBresenham int:   %f secs\nBresenham opt:   %f secs", naive_time, fixed_point_time, float_bresenham_time, int_bresenham_time, opt_bresenham_time);
		int32_t text_width, text_height;
		r96_font_get_text_bounds(&font, text, &text_width, &text_height);
		r96_rect(&output, 0, 0, text_width, text_height, 0xff222222);
		r96_text(&output, &font, text, 0, 0, 0xffffffff);
		if (mfb_update_ex(window, output.pixels, output.width, output.height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
