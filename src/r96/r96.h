#pragma once

#include <stdlib.h>
#include <stdint.h>

#define R96_ARGB(alpha, red, green, blue) (((uint8_t) alpha << 24) | ((uint8_t) red << 16) | ((uint8_t) green << 8) | (uint8_t) blue)
#define R96_A(color) ((uint8_t) (color >> 24))
#define R96_R(color) ((uint8_t) (color >> 16))
#define R96_G(color) ((uint8_t) (color >> 8))
#define R96_B(color) ((uint8_t) (color))

#define R96_ALLOC(type) (type *) malloc(sizeof(type))
#define R96_ALLOC_ARRAY(type, numElements) (type *) malloc(sizeof(type) * numElements)
#define R96_FREE(ptr) free(ptr)

typedef struct r96_raster_image {
	int32_t width, height;
	uint32_t *pixels;
} r96_image;

void r96_image_init(r96_image *image, int32_t width, int32_t height);

void r96_image_dispose(r96_image *image);

void r96_clear(r96_image *image);

void r96_clear_with_color(r96_image *image, uint32_t color);

void r96_set_pixel(r96_image *image, int32_t x, int32_t y, uint32_t color);

uint32_t r96_get_pixel(r96_image *image, int32_t x, int32_t y);

void r96_hline(r96_image *image, int32_t x1, int32_t x2, int32_t y, uint32_t color);

void r96_rect(r96_image *image, int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color);
