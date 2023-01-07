#include <MiniFB.h>
#include <stdlib.h>
#include "r96/r96.h"

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("13_blit_region", window_width, window_height);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/tamzen.png", 8, 16);
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		for (int y = 0; y < font.glyphs.height; y += font.glyph_height) {
			for (int x = 0; x < font.glyphs.width; x += font.glyph_width) {
				r96_blit_region(&output, &font.glyphs, x, y, x, y, font.glyph_width, font.glyph_height);
				r96_blit_region_keyed(&output, &font.glyphs, x + 15 * font.glyph_width, y, x, y, font.glyph_width, font.glyph_height, 0x0);
				r96_blit_region_keyed_tinted(&output, &font.glyphs, x + 30 * font.glyph_width, y, x, y, font.glyph_width, font.glyph_height, 0x0, 0xff00ff);
			}
		}

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
