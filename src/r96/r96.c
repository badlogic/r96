#include "r96.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "stb_image.h"

#define UNUSED(x) ((void) x)

uint32_t r96_next_utf8_code_point(const char *data, uint32_t *index, uint32_t end) {
	static const uint32_t utf8_offsets[6] = {
			0x00000000UL, 0x00003080UL, 0x000E2080UL,
			0x03C82080UL, 0xFA082080UL, 0x82082080UL};

	uint32_t character = 0;
	const unsigned char *bytes = (const unsigned char *) data;
	int num_bytes = 0;
	do {
		character <<= 6;
		character += bytes[(*index)++];
		num_bytes++;
	} while (*index != end && ((bytes[*index]) & 0xC0) == 0x80);
	character -= utf8_offsets[num_bytes - 1];

	return character;
}

void r96_byte_buffer_init(r96_byte_buffer *buffer, size_t num_bytes) {
	buffer->num_bytes = num_bytes;
	buffer->bytes = R96_ALLOC_ARRAY(uint8_t, num_bytes);
}

#ifndef __EMSCRIPTEN__
bool r96_byte_buffer_init_from_file(r96_byte_buffer *buffer, const char *path) {
	*buffer = (r96_byte_buffer){0};
	FILE *file = fopen(path, "rb");
	if (!file) return false;

	if (fseek(file, 0, SEEK_END)) goto _error;
	long int num_bytes = ftell(file);
	if (num_bytes == -1) goto _error;
	if (fseek(file, 0, SEEK_SET)) goto _error;

	buffer->num_bytes = num_bytes;
	buffer->bytes = R96_ALLOC_ARRAY(uint8_t, buffer->num_bytes);
	if (fread(buffer->bytes, sizeof(uint8_t), buffer->num_bytes, file) != buffer->num_bytes) goto _error;

	fclose(file);
	return true;

_error:
	fclose(file);
	if (buffer->bytes) R96_FREE(buffer->bytes);
	buffer->num_bytes = 0;
	return false;
}
#else
#include <emscripten.h>

EM_ASYNC_JS(uint8_t *, load_file, (const char *path, size_t *size), {
	let url = "./" + UTF8ToString(path);
	let response = await fetch(url);
	if (!response.ok) return 0;
	let data = new Uint8Array(await response.arrayBuffer());
	let ptr = _malloc(data.byteLength);
	HEAPU8.set(data, ptr);
	HEAPU32[size >> 2] = data.byteLength;
	return ptr;
})

bool r96_byte_buffer_init_from_file(r96_byte_buffer *buffer, const char *path) {
	buffer->bytes = load_file(path, &buffer->num_bytes);
	return buffer->bytes != NULL;
}
#endif

void r96_byte_buffer_dispose(r96_byte_buffer *buffer) {
	R96_FREE(buffer->bytes);
}

void r96_image_init(r96_image *image, int32_t width, int32_t height) {
	assert(width > 0);
	assert(height > 0);

	image->width = width;
	image->height = height;
	image->pixels = R96_ALLOC_ARRAY(uint32_t, width * height);
}

bool r96_image_init_from_file(r96_image *image, const char *path) {
	r96_byte_buffer buffer;
	if (!r96_byte_buffer_init_from_file(&buffer, path)) return false;
	image->pixels = (uint32_t *) stbi_load_from_memory(buffer.bytes, (int) buffer.num_bytes, (int *) &image->width, (int *) &image->height, NULL, 4);
	r96_byte_buffer_dispose(&buffer);
	if (image->pixels == NULL) return false;

	uint8_t *bytes = (uint8_t *) image->pixels;
	int32_t n = image->width * image->height * sizeof(uint32_t);
	for (int32_t i = 0; i < n; i += 4) {
		uint8_t b = bytes[i];
		bytes[i] = bytes[i + 2];
		bytes[i + 2] = b;
	}
	return true;
}

void r96_image_dispose(r96_image *image) {
	R96_FREE(image->pixels);
}

bool r96_font_init(r96_font *font, const char *path, int32_t glyph_width, int32_t glyph_height) {
	if (!r96_image_init_from_file(&font->glyph_atlas, path)) return false;
	font->glyph_width = glyph_width;
	font->glyph_height = glyph_height;
	font->glyphs_per_row = font->glyph_atlas.width / glyph_width;
	font->tab_size = 3;
	return true;
}

void r96_font_dispose(r96_font *font) {
	r96_image_dispose(&font->glyph_atlas);
}

void r96_font_get_text_bounds(r96_font *font, const char *text, int32_t *width, int32_t *height) {
	*width = 0;
	*height = font->glyph_height;
	int32_t current_line_width = 0;
	uint32_t text_length = strlen(text);
	uint32_t index = 0;
	while (index < text_length) {
		uint32_t c = r96_next_utf8_code_point(text, &index, text_length);
		if (c == '\t') {
			current_line_width += font->tab_size * font->glyph_width;
			continue;
		}
		if (c == '\n') {
			*width = current_line_width > *width ? current_line_width : *width;
			*height += font->glyph_height;
			current_line_width = 0;
			continue;
		}
		current_line_width += font->glyph_width;
	}
	*width = current_line_width > *width ? current_line_width : *width;
}

void r96_clear(r96_image *image) {
	memset(image->pixels, 0x0, image->width * image->height * sizeof(uint32_t));
}

void r96_clear_with_color(r96_image *image, uint32_t color) {
	uint32_t *pixels = image->pixels;
	for (int32_t i = 0, n = image->width * image->height; i < n; i++)
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
	for (int32_t y = y1; y <= y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			*pixel++ = color;
		}
		pixel += next_row;
	}
}

void r96_blit(r96_image *dst, r96_image *src, int32_t x, int32_t y) {
	int32_t dst_x1 = x;
	int32_t dst_y1 = y;
	int32_t dst_x2 = x + src->width - 1;
	int32_t dst_y2 = y + src->height - 1;
	int32_t src_x1 = 0;
	int32_t src_y1 = 0;

	if (dst_x1 >= dst->width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= dst->height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= dst->width) dst_x2 = dst->width - 1;
	if (dst_y2 >= dst->height) dst_y2 = dst->height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = dst->width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint32_t *dst_pixel = dst->pixels + dst_y1 * dst->width + dst_x1;
	uint32_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (y = dst_y1; y <= dst_y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void r96_blit_keyed(r96_image *dst, r96_image *src, int32_t x, int32_t y, uint32_t color_key) {
	int32_t dst_x1 = x;
	int32_t dst_y1 = y;
	int32_t dst_x2 = x + src->width - 1;
	int32_t dst_y2 = y + src->height - 1;
	int32_t src_x1 = 0;
	int32_t src_y1 = 0;

	if (dst_x1 >= dst->width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= dst->height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= dst->width) dst_x2 = dst->width - 1;
	if (dst_y2 >= dst->height) dst_y2 = dst->height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = dst->width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint32_t *dst_pixel = dst->pixels + dst_y1 * dst->width + dst_x1;
	uint32_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (y = dst_y1; y <= dst_y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			uint32_t src_color = *src_pixel;
			uint32_t dst_color = *dst_pixel;
			*dst_pixel = src_color != color_key ? src_color : dst_color;
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void r96_blit_region(r96_image *dst, r96_image *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height) {
	assert(src_x + src_width - 1 < src->width);
	assert(src_y + src_height - 1 < src->height);

	int32_t dst_x1 = dst_x;
	int32_t dst_y1 = dst_y;
	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;
	int32_t src_x1 = src_x;
	int32_t src_y1 = src_y;

	if (dst_x1 >= dst->width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= dst->height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= dst->width) dst_x2 = dst->width - 1;
	if (dst_y2 >= dst->height) dst_y2 = dst->height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = dst->width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint32_t *dst_pixel = dst->pixels + dst_y1 * dst->width + dst_x1;
	uint32_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (int32_t y = dst_y1; y <= dst_y2; y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void r96_blit_region_keyed(r96_image *dst, r96_image *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint32_t color_key) {
	assert(src_x + src_width - 1 < src->width);
	assert(src_y + src_height - 1 < src->height);

	int32_t dst_x1 = dst_x;
	int32_t dst_y1 = dst_y;
	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;
	int32_t src_x1 = src_x;
	int32_t src_y1 = src_y;

	if (dst_x1 >= dst->width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= dst->height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= dst->width) dst_x2 = dst->width - 1;
	if (dst_y2 >= dst->height) dst_y2 = dst->height - 1;

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = dst->width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint32_t *dst_pixel = dst->pixels + dst_y1 * dst->width + dst_x1;
	uint32_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (dst_y = dst_y1; dst_y <= dst_y2; dst_y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			uint32_t src_color = *src_pixel;
			uint32_t dst_color = *dst_pixel;
			*dst_pixel = src_color != color_key ? src_color : dst_color;
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void r96_blit_region_keyed_tinted(r96_image *dst, r96_image *src, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height, uint32_t color_key, uint32_t tint) {
	assert(src_x + src_width - 1 < src->width);
	assert(src_y + src_height - 1 < src->height);

	int32_t dst_x1 = dst_x;
	int32_t dst_y1 = dst_y;
	int32_t dst_x2 = dst_x + src_width - 1;
	int32_t dst_y2 = dst_y + src_height - 1;
	int32_t src_x1 = src_x;
	int32_t src_y1 = src_y;

	if (dst_x1 >= dst->width) return;
	if (dst_x2 < 0) return;
	if (dst_y1 >= dst->height) return;
	if (dst_y2 < 0) return;

	if (dst_x1 < 0) {
		src_x1 -= dst_x1;
		dst_x1 = 0;
	}
	if (dst_y1 < 0) {
		src_y1 -= dst_y1;
		dst_y1 = 0;
	}
	if (dst_x2 >= dst->width) dst_x2 = dst->width - 1;
	if (dst_y2 >= dst->height) dst_y2 = dst->height - 1;

	uint32_t tint_r = R96_R(tint);
	uint32_t tint_g = R96_G(tint);
	uint32_t tint_b = R96_B(tint);

	int32_t clipped_width = dst_x2 - dst_x1 + 1;
	int32_t dst_next_row = dst->width - clipped_width;
	int32_t src_next_row = src->width - clipped_width;
	uint32_t *dst_pixel = dst->pixels + dst_y1 * dst->width + dst_x1;
	uint32_t *src_pixel = src->pixels + src_y1 * src->width + src_x1;
	for (dst_y = dst_y1; dst_y <= dst_y2; dst_y++) {
		for (int32_t i = 0; i < clipped_width; i++) {
			uint32_t src_color = *src_pixel;
			uint32_t dst_color = *dst_pixel;
			*dst_pixel = src_color != color_key ? R96_ARGB(
														  R96_A(src_color),
														  ((R96_R(src_color) * tint_r) >> 8) & 0xff,
														  ((R96_G(src_color) * tint_g) >> 8) & 0xff,
														  ((R96_B(src_color) * tint_b) >> 8) & 0xff)
												: dst_color;
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void r96_text(r96_image *image, r96_font *font, const char *text, int32_t x, int32_t y, uint32_t tint) {
	int32_t cursor_x = x;
	int32_t cursor_y = y;
	uint32_t text_length = strlen(text);
	uint32_t index = 0;
	while (index < text_length) {
		uint32_t c = r96_next_utf8_code_point(text, &index, text_length);
		if (c == '\t') {
			cursor_x += font->tab_size * font->glyph_width;
			continue;
		}
		if (c == '\n') {
			cursor_x = x;
			cursor_y += font->glyph_height;
			continue;
		}
		if (c < 32 || c > 255) {
			cursor_x += font->glyph_width;
			continue;
		}

		int32_t glyph_index = c - 32;
		int32_t glyph_x = (glyph_index % font->glyphs_per_row);
		int32_t glyph_y = (glyph_index - glyph_x) / font->glyphs_per_row;
		glyph_x *= font->glyph_width;
		glyph_y *= font->glyph_height;

		r96_blit_region_keyed_tinted(image, &font->glyph_atlas, cursor_x, cursor_y, glyph_x, glyph_y, font->glyph_width, font->glyph_height, 0x0, tint);

		cursor_x += font->glyph_width;
	}
}
