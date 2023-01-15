#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define R96_ARGB(alpha, red, green, blue) (uint32_t)(((uint8_t) (alpha) << 24) | ((uint8_t) (red) << 16) | ((uint8_t) (green) << 8) | (uint8_t) (blue))
#define R96_A(color) ((uint8_t) (color >> 24))
#define R96_R(color) ((uint8_t) (color >> 16))
#define R96_G(color) ((uint8_t) (color >> 8))
#define R96_B(color) ((uint8_t) (color))

#define R96_ALLOC(type) (type *) malloc(sizeof(type))
#define R96_ALLOC_ARRAY(type, num_elements) (type *) malloc(sizeof(type) * num_elements)
#define R96_FREE(ptr) free(ptr)

typedef struct r96_image {
	int32_t width, height;
	uint32_t *pixels;
} r96_image;

typedef struct r96_byte_buffer {
	uint8_t *bytes;
	size_t num_bytes;
} r96_byte_buffer;

typedef struct r96_font {
	r96_image glyph_atlas;
	int32_t glyph_width, glyph_height;
	int32_t glyphs_per_row;
	int32_t tab_size;
} r96_font;

uint32_t r96_next_utf8_code_point(const char *data, uint32_t *index, uint32_t end);

void r96_byte_buffer_init(r96_byte_buffer *buffer, size_t size);

bool r96_byte_buffer_init_from_file(r96_byte_buffer *buffer, const char *path);

void r96_byte_buffer_dispose(r96_byte_buffer *buffer);

void r96_image_init(r96_image *image, int32_t width, int32_t height);

bool r96_image_init_from_file(r96_image *image, const char *path);

void r96_image_dispose(r96_image *image);

bool r96_font_init(r96_font *font, const char *path, int32_t glyph_width, int32_t glyph_height);

void r96_font_dispose(r96_font *font);

void r96_font_get_text_bounds(r96_font *font, const char *text, int32_t *width, int32_t *height);

void r96_clear(r96_image *image);

void r96_clear_with_color(r96_image *image, uint32_t color);

void r96_set_pixel(r96_image *image, int32_t x, int32_t y, uint32_t color);

uint32_t r96_get_pixel(r96_image *image, int32_t x, int32_t y);

void r96_hline(r96_image *image, int32_t x1, int32_t x2, int32_t y, uint32_t color);

void r96_vline(r96_image *image, int32_t x, int32_t y1, int32_t y2, uint32_t color);

void r96_rect(r96_image *image, int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color);

void r96_blit(r96_image *dst, r96_image *src, int32_t x, int32_t y);

void r96_blit_keyed(r96_image *dst, r96_image *src, int32_t x, int32_t y, uint32_t color_key);

void r96_blit_region(r96_image *dst, r96_image *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height);

void r96_blit_region_keyed(r96_image *dst, r96_image *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint32_t color_key);

void r96_blit_region_keyed_tinted(r96_image *dst, r96_image *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint32_t color_key, uint32_t tint);

void r96_text(r96_image *image, r96_font *font, const char *text, int32_t x, int32_t y, uint32_t tint);
