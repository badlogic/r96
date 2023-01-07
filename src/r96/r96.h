#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define R96_ARGB(alpha, red, green, blue) (((uint8_t) alpha << 24) | ((uint8_t) red << 16) | ((uint8_t) green << 8) | (uint8_t) blue)
#define R96_A(color) ((uint8_t) (color >> 24))
#define R96_R(color) ((uint8_t) (color >> 16))
#define R96_G(color) ((uint8_t) (color >> 8))
#define R96_B(color) ((uint8_t) (color))

#define R96_ALLOC(type) (type *) malloc(sizeof(type))
#define R96_ALLOC_ARRAY(type, num_elements) (type *) malloc(sizeof(type) * num_elements)
#define R96_FREE(ptr) free(ptr)

typedef struct r96_raster_image {
	int32_t width, height;
	uint32_t *pixels;
} r96_image;

typedef struct r96_byte_buffer {
	uint8_t *bytes;
	size_t num_bytes;
} r96_byte_buffer;

void r96_byte_buffer_init(r96_byte_buffer *buffer, size_t size);

bool r96_byte_buffer_init_from_file(r96_byte_buffer *buffer, const char *path);

void r96_byte_buffer_dispose(r96_byte_buffer *buffer);

void r96_image_init(r96_image *image, int32_t width, int32_t height);

bool r96_image_init_from_file(r96_image *image, const char *path);

void r96_image_dispose(r96_image *image);

void r96_clear(r96_image *image);

void r96_clear_with_color(r96_image *image, uint32_t color);

void r96_set_pixel(r96_image *image, int32_t x, int32_t y, uint32_t color);

uint32_t r96_get_pixel(r96_image *image, int32_t x, int32_t y);

void r96_hline(r96_image *image, int32_t x1, int32_t x2, int32_t y, uint32_t color);

void r96_rect(r96_image *image, int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color);

void r96_blit(r96_image *dst, r96_image *src, int x, int y);

void r96_blit_keyed(r96_image *dst, r96_image *src, int x, int y, uint32_t color_key);
