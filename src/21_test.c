#include <MiniFB.h>
#include <math.h>
#include <stdio.h>
#include "MiniFB_enums.h"
#include "r96/r96.h"
#include <assert.h>

int32_t fixed_one(int32_t bits) {
	return 1 << bits;
}

int32_t fixed_one_half(int32_t bits) {
	return 1 << (bits - 1);
}

int32_t float_to_fixed(float v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

int32_t int_to_fixed(int32_t v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

int32_t fixed_to_int(int32_t v, int32_t bits) {
	return v >> bits;
}

float fixed_to_float(int32_t v, int32_t bits) {
	return v / (float) (1 << bits);
}

int32_t fixed_div(int32_t a, int32_t b, int32_t bits) {
	return ((int64_t) a * (1 << bits)) / b;
}

int32_t fixed_mul(int32_t a, int32_t b, int32_t bits) {
	return (int32_t) (((int64_t) a * (int64_t) b) >> bits);
}

int32_t fixed_floor(int32_t v, int32_t bits) {
	int32_t fpOne = (1 << bits);
	int32_t subMask = fpOne - 1;
	return (v & ~subMask);
}

float quantize(float v, int bits) {
	return fixed_to_float(float_to_fixed(v, bits), bits);
}

void line_naive(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
	int32_t delta_x = (x2 - x1);
	int32_t delta_y = (y2 - y1);
	int32_t num_pixels_x = abs(delta_x) + 1;
	int32_t num_pixels_y = abs(delta_y) + 1;
	int32_t pixel_size = image->width / 32;

	float step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = delta_x < 0 ? -1 : 1;
		step_y = (float) delta_y / abs(delta_x);
		num_pixels = num_pixels_x;
	} else {
		step_x = (float) delta_x / abs(delta_y);
		step_y = delta_y < 0 ? -1 : 1;
		num_pixels = num_pixels_y;
	}

	float x = x1 + 0.5f, y = y1 + 0.5f;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_rect(image, (int32_t) x * pixel_size, (int32_t) y * pixel_size, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
		r96_set_pixel(image, x * pixel_size, y * pixel_size, 0xffff00ff);
		x += step_x;
		y += step_y;
	}
}

void line_naive_non_incremental(r96_image *image, float x1, float y1, float x2, float y2, int bits) {
	x1 = quantize(x1, bits);
	y1 = quantize(y1, bits);
	x2 = quantize(x2, bits);
	y2 = quantize(y2, bits);
	float delta_x = (x2 - x1);
	float delta_y = (y2 - y1);
	float num_pixels_x = fabs(floorf(x2) - floorf(x1)) + 1;
	float num_pixels_y = fabs(floorf(y2) - floorf(y1)) + 1;
	float pixel_size = image->width / 32;

	uint32_t num_pixels = num_pixels_x > num_pixels_y ? num_pixels_x : num_pixels_y;
	for (uint32_t i = 0; i < num_pixels; i++) {
		float x = x1 + (delta_x / (num_pixels - 1)) * i;
		float y = y1 + (delta_y / (num_pixels - 1)) * i;

		r96_rect(image, (int32_t) x * pixel_size, (int32_t) y * pixel_size, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
		r96_set_pixel(image, x * pixel_size, y * pixel_size, 0xffff00ff);
	}
}

void line_naive_offset_incremental_fixed(r96_image *image, float i_x1, float i_y1, float i_x2, float i_y2, int bits) {
	int32_t x1 = float_to_fixed(i_x1, bits);
	int32_t y1 = float_to_fixed(i_y1, bits);
	int32_t x2 = float_to_fixed(i_x2, bits);
	int32_t y2 = float_to_fixed(i_y2, bits);
	int32_t delta_x = (x2 - x1);
	int32_t delta_y = (y2 - y1);
	int32_t num_pixels_x = fabs(floorf(i_x2) - floorf(i_x1)) + 1;
	int32_t num_pixels_y = fabs(floorf(i_y2) - floorf(i_y1)) + 1;
	int32_t pixel_size = image->width / 32;

	uint32_t num_pixels = num_pixels_x > num_pixels_y ? num_pixels_x : num_pixels_y;
	if (num_pixels == 1) {
		r96_set_pixel(image, fixed_to_int(x1, bits), fixed_to_int(y1, bits), 0xffff00ff);
		return;
	}

	int32_t x = 0, y = 0;
	int32_t step_x = 0;
	int32_t step_y = 0;
	if (num_pixels_x >= num_pixels_y) {
		x = (fixed_floor(x1, bits) + fixed_one_half(bits));
		step_x = fixed_one(bits);
		step_y = fixed_div(delta_y, abs(delta_x), bits);
		y = y1 + fixed_mul(x - x1, step_y, bits);
	} else {
		y = (fixed_floor(y1, bits) + fixed_one_half(bits));
		step_y = fixed_one(bits);
		step_x = fixed_div(delta_x, abs(delta_y), bits);
		x = x1 + fixed_mul(y - y1, step_x, bits);
	}

	for (uint32_t i = 0; i < num_pixels; i++) {
		int px = fixed_to_int(x, bits), py = fixed_to_int(y, bits);
		r96_rect(image, px * pixel_size, py * pixel_size, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
		r96_set_pixel(image, fixed_to_float(x, bits) * pixel_size, fixed_to_float(y, bits) * pixel_size, 0xffff00ff);
		//r96_set_pixel(image, 0xffff00ff);
		x += step_x;
		y += step_y;
	}
}

void line_quantized(r96_image *image, float x1, float y1, float x2, float y2, int bits) {
	x1 = quantize(x1, bits);
	y1 = quantize(y1, bits);
	x2 = quantize(x2, bits);
	y2 = quantize(y2, bits);
	float delta_x = (x2 - x1);
	float delta_y = (y2 - y1);
	int32_t num_pixels_x = fabs(floorf(x2) - floorf(x1));
	int32_t num_pixels_y = fabs(floorf(y2) - floorf(y1));
	int32_t pixel_size = image->width / 32;

	float step_x, step_y;
	uint32_t num_pixels = num_pixels_x > num_pixels_y ? num_pixels_x : num_pixels_y;
	float one_over_num_pixels = 1.0f / num_pixels;
	step_x = delta_x * one_over_num_pixels;
	step_y = delta_y * one_over_num_pixels;
	float x = x1, y = y1;

	printf("-- start\n");
	printf("stx: %f sty: %f\n", x1, y1);
	printf("np: %i\n", num_pixels);
	printf("sx: %f sy: %f\n", step_x, step_y);

	for (uint32_t i = 0; i <= num_pixels; i++) {
		printf("x: %f y: %f\n", x, y);
		r96_rect(image, (int32_t) x * pixel_size, (int32_t) y * pixel_size, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
		uint32_t px = (int32_t) (x * pixel_size);
		uint32_t py = (int32_t) (y * pixel_size);
		r96_set_pixel(image, px, py, 0xffff00ff);
		printf("px: %i py: %i\n", px, py);
		x += step_x;
		y += step_y;
	}
	printf("tx: %f ty: %f\n", x2, y2);
	printf("-- end\n");
}

void line_quantized2(r96_image *image, float x1, float y1, float x2, float y2, int bits) {
	x1 = quantize(x1, bits);
	y1 = quantize(y1, bits);
	x2 = quantize(x2, bits);
	y2 = quantize(y2, bits);
	float delta_x = (x2 - x1);
	float delta_y = (y2 - y1);
	int32_t num_pixels_x = fabs(floorf(x2) - floorf(x1)) + 1;
	int32_t num_pixels_y = fabs(floorf(y2) - floorf(y1)) + 1;
	int32_t pixel_size = image->width / 32;

	float step_x, step_y;
	uint32_t num_pixels = num_pixels_x > num_pixels_y ? num_pixels_x : num_pixels_y;
	if (fabs(delta_x) > fabs(delta_y)) {
		step_x = delta_x < 0 ? -1 : 1;
		step_y = delta_y / fabs(delta_x);
		num_pixels = num_pixels_x;
		// if ((int32_t) (x1 + (num_pixels - 1) * step_x) == (int32_t) x2) num_pixels--;
		printf("x-major\n");
	} else {
		step_x = delta_x / fabs(delta_y);
		step_y = delta_y < 0 ? -1 : 1;
		num_pixels = num_pixels_y;
		// if ((int32_t) (y1 + (num_pixels - 1) * step_y) == (int32_t) y2) num_pixels--;
		printf("y-major\n");
	}
	float x = x1, y = y1;

	printf("-- start\n");
	printf("stx: %f sty: %f\n", x1, y1);
	printf("np: %i\n", num_pixels);
	printf("sx: %f sy: %f\n", step_x, step_y);

	uint32_t i = 0;
	for (; i < num_pixels; i++) {
		printf("x: %f y: %f\n", x, y);
		r96_rect(image, (int32_t) x * pixel_size, (int32_t) y * pixel_size, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
		uint32_t px = (int32_t) (x * pixel_size);
		uint32_t py = (int32_t) (y * pixel_size);
		r96_set_pixel(image, px, py, 0xffff00ff);
		printf("px: %i py: %i\n", px, py);
		x += step_x;
		y += step_y;
	}
	r96_rect(image, (int32_t) x2 * pixel_size, (int32_t) y2 * pixel_size, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
	uint32_t px = (int32_t) (x2 * pixel_size);
	uint32_t py = (int32_t) (y2 * pixel_size);
	r96_set_pixel(image, px, py, 0xffff00ff);
	printf("tx: %f ty: %f\n", x2, y2);
	printf("-- end\n");
}

void line_sub_pixel(r96_image *image, float x1, float y1, float x2, float y2, int bits) {
	int32_t x1_fp = float_to_fixed(x1, bits);
	int32_t y1_fp = float_to_fixed(y1, bits);
	int32_t x2_fp = float_to_fixed(x2, bits);
	int32_t y2_fp = float_to_fixed(y2, bits);
	int32_t delta_x = (x2_fp - x1_fp);
	int32_t delta_y = (y2_fp - y1_fp);
	int32_t num_pixels_x = fixed_floor(abs(delta_x), bits);
	int32_t num_pixels_y = fixed_floor(abs(delta_y), bits);
	int32_t pixel_size = image->width / 32;
	int32_t fixed_pixel_size = int_to_fixed(pixel_size, bits);

	int32_t num_pixels = num_pixels_x > num_pixels_y ? num_pixels_x : num_pixels_y;
	int32_t step_x = fixed_div(delta_x, num_pixels, bits);
	int32_t step_y = fixed_div(delta_y, num_pixels, bits);
	num_pixels = fixed_to_int(num_pixels, bits);

	int32_t x = x1_fp, y = y1_fp;
	for (int32_t i = 0; i <= num_pixels; i++) {
		int32_t px = fixed_to_int(x * fixed_pixel_size, bits);
		int32_t py = fixed_to_int(y * fixed_pixel_size, bits);
		int32_t rx = fixed_to_int(x, bits) * pixel_size;
		int32_t ry = fixed_to_int(y, bits) * pixel_size;
		r96_rect(image, rx, ry, pixel_size, pixel_size, i % 2 ? 0xffbbbbbb : 0xff555555);
		r96_set_pixel(image, px / pixel_size, py / pixel_size, 0xffff00ff);
		x += step_x;
		y += step_y;
	}
}


void test(float x1, float y1, float x2, float y2) {
	const int window_width = 32 * 50, window_height = 24 * 50;
	struct mfb_window *window = mfb_open("19_sub_pixel_line", window_width, window_height);
	r96_image output;
	int bits = 3;
	int pixel_size = 1 << bits;
	r96_image_init(&output, 32 * pixel_size, 24 * pixel_size);
	int mode = 2;
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));
		float qx1 = quantize(x1, bits);
		float qy1 = quantize(y1, bits);
		float qx2 = quantize(x2, bits);
		float qy2 = quantize(y2, bits);
		uint32_t px1 = quantize(x1, bits) * pixel_size;
		uint32_t py1 = quantize(y1, bits) * pixel_size;
		uint32_t px2 = quantize(x2, bits) * pixel_size;
		uint32_t py2 = quantize(y2, bits) * pixel_size;
		printf("x1: %f y1: %f\n", x1, y1);
		printf("x2: %f y2: %f\n", x2, y2);
		printf("qx1: %f qy1: %f\n", qx1, qy1);
		printf("qx2: %f qy2: %f\n", qx2, qy2);
		printf("px1: %i py1: %i\n", px1, py1);
		printf("px2: %i py2: %i\n", px2, py2);
		if (mode == 0) line_naive(&output, x1, y1, x2, y2);
		if (mode == 1) line_naive_offset_incremental_fixed(&output, x1, y1, x2, y2, bits);
		// if (mode == 3) line_sub_pixel(&output, x1, y1, x2, y2, bits);
		//if (mode == 2) line_quantized2(&output, x1, y1, x2, y2, bits);
		if (mode == 2) line_naive_non_incremental(&output, x1, y1, x2, y2, bits);

		r96_set_pixel(&output, px1, py1, 0xff0000ff);
		r96_set_pixel(&output, px2, py2, 0xff00ff00);

		if (mfb_get_mouse_button_buffer(window)[MOUSE_LEFT]) {
			x2 = mfb_get_mouse_x(window) / ((float) window_width) * output.width / pixel_size;
			y2 = mfb_get_mouse_y(window) / ((float) window_height) * output.height / pixel_size;
		}
		if (mfb_get_mouse_button_buffer(window)[MOUSE_RIGHT]) {
			x1 = mfb_get_mouse_x(window) / ((float) window_width) * output.width / pixel_size;
			y1 = mfb_get_mouse_y(window) / ((float) window_height) * output.height / pixel_size;
		}

		if (mfb_get_key_buffer(window)[KB_KEY_1]) mode = 0;
		if (mfb_get_key_buffer(window)[KB_KEY_2]) mode = 1;
		if (mfb_get_key_buffer(window)[KB_KEY_3]) mode = 2;

		if (mfb_update_ex(window, output.pixels, output.width, output.height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
}

int main(void) {
	//test(0.355347, 0.000000, 8.037109, 18.406219);
	//test(0.355347, 0.000000, 1.260000, 0.500000);
	// Doesn't hit end point due to floating point precision
	// test(0.355347, 0.000000, 6.300000, 2.000000);
	// test(0.355347, 0.000000, 10.760000, 11.900000);
	// test(0.355347, 0.000000, 11.000000, 11.660000);
	// test(16.559999, 9.500000, 22.059999, 5.440000);
	// test(16.559999, 9.500000, 23.420000, 5.100000);
	test(128.906631 - 120, 77.783615 - 70, 120.145752 - 120, 70.885635 - 70);
	// test(134.541977 - 134, 72.447189 - 68, 146.043823 - 134, 70.098663 - 68);
	return 0;
}
