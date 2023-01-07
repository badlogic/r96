#include <MiniFB.h>
#include <stdlib.h>
#include "r96/r96.h"

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("14_fonts", window_width, window_height);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/ibmvga.png", 8, 16);

	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		const char *text = "The quick brown fox jumps\nover the lazy dog\n"
						   "¡¢£¤¥¦§¨©ª«¬\n"
						   "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ";

		int32_t text_x, text_y, text_width, text_height;
		r96_font_get_text_bounds(&font, text, &text_width, &text_height);
		text_x = window_width / 2 - text_width / 2;
		text_y = window_height / 2 - text_height / 2;

		r96_rect(&output, text_x, text_y, text_width, text_height, R96_ARGB(0xff, 0xff, 0x0, 0xff));
		r96_text(&output, &font, text, text_x + 1, text_y + 1, 0x00000000);
		r96_text(&output, &font, text, text_x, text_y, 0xffffffff);

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
