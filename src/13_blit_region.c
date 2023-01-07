#include <MiniFB.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "r96/r96.h"

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("13_blit_region", window_width, window_height);
	r96_image output;
	r96_image_init(&output, window_width, window_height);

	r96_image glyph_atlas;
	int32_t glyph_width = 8;
	int32_t glyph_height = 16;
	int32_t glyphs_per_row = 16;
	r96_image_init_from_file(&glyph_atlas, "assets/ibmvga.png");

	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		const char *text = "Hello world!";
		uint32_t text_length = strlen(text);
		uint32_t char_index = 0;
		uint32_t x_offset = 100;
		while (char_index < text_length) {
			uint32_t code_point = r96_next_utf8_code_point(text, &char_index, text_length);
			int32_t glyph_x = (code_point - 32) % glyphs_per_row;
			int32_t glyph_y = (code_point - 32 - glyph_x) / glyphs_per_row;
			int32_t glyph_pixel_x = glyph_x * glyph_width;
			int32_t glyph_pixel_y = glyph_y * glyph_height;

			r96_blit_region(&output, &glyph_atlas, x_offset, 50, glyph_pixel_x, glyph_pixel_y, glyph_width, glyph_height);
			r96_blit_region_keyed(&output, &glyph_atlas, x_offset, 100, glyph_pixel_x, glyph_pixel_y, glyph_width, glyph_height, 0x0);
			r96_blit_region_keyed_tinted(&output, &glyph_atlas, x_offset, 150, glyph_pixel_x, glyph_pixel_y, glyph_width, glyph_height, 0x0, 0xffff00ff);
			x_offset += glyph_width;
		}

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
