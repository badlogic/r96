#include <MiniFB.h>
#include <math.h>
#include "r96/r96.h"

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
	const int window_width = 40, window_height = 30;
	int scale = 16;
	struct mfb_window *window = mfb_open("20_bresenham_opt", window_width * scale, window_height * scale);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));
		int32_t mouse_x = mfb_get_mouse_x(window) / scale;
		int32_t mouse_y = mfb_get_mouse_y(window) / scale;
		line_optimized_bresenham(&output, window_width / 2, window_height / 2, mouse_x, mouse_y, 0xffff0000);

		if (mfb_update_ex(window, output.pixels, output.width, output.height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
