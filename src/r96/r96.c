#include "r96.h"
#include <assert.h>
#include <string.h>

void r96_image_init(r96_image *image, int32_t width, int32_t height) {
	assert(width > 0);
	assert(height > 0);

	image->width = width;
	image->height = height;
	image->pixels = R96_ALLOC_ARRAY(uint32_t, width * height);
}

void r96_image_dispose(r96_image *image) {
	R96_FREE(image->pixels);
}

void r96_clear(r96_image *image) {
	memset(image->pixels, 0x0, image->width * image->height * sizeof(uint32_t));
}

void r96_clear_with_color(r96_image *image, uint32_t color) {
	uint32_t *pixels = image->pixels;
	for (int i = 0, n = image->width * image->height; i < n; i++)
		pixels[i] = color;
}

void r96_set_pixel(r96_image *image, int32_t x, int32_t y, uint32_t color) {
	if (x < 0 || x >= image->width || y < 0 || y >= image->height) return;
	image->pixels[x + y * image->width] = color;
}

uint32_t r96_get_pixel(r96_image *image, int32_t x, int32_t y) {
	if (x < 0 || x >= image->width || y < 0 || y >= image->height) return 0;
	return image->pixels[x + y * image->width];
}

void r96_hline(r96_image *image, int32_t x1, int32_t x2, int32_t y, uint32_t color) {
	if (x1 > x2) {
		int32_t tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	if (x1 >= image->width) return;
	if (x2 < 0) return;
	if (y < 0) return;
	if (y >= image->height) return;

	if (x1 < 0) x1 = 0;
	if (x2 >= image->width) x2 = image->width - 1;

	uint32_t *pixels = image->pixels + y * image->width + x1;
	int32_t num_pixels = x2 - x1 + 1;
	while (num_pixels--) {
		*pixels++ = color;
	}
}

void r96_rect(r96_image *image, int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color) {
	if (width <= 0) return;
	if (height <= 0) return;

	int32_t x2 = x1 + width - 1;
	int32_t y2 = y1 + height - 1;

	if (x1 >= image->width) return;
	if (x2 < 0) return;
	if (y1 >= image->height) return;
	if (y2 < 0) return;

	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 >= image->width) x2 = image->width - 1;
	if (y2 >= image->height) y2 = image->height - 1;

	int32_t clipped_width = x2 - x1 + 1;
	int32_t next_row = image->width - clipped_width;
	uint32_t *pixel = image->pixels + y1 * image->width + x1;
	for (int y = y1; y <= y2; y++) {
		for (int i = 0; i < clipped_width; i++) {
			*pixel++ = color;
		}
		pixel += next_row;
	}
}
