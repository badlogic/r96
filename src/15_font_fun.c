#include <MiniFB.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "r96/r96.h"

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("15_font_fun", window_width, window_height);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/demofont.png", 16, 16);
	float counter = 0;
	struct mfb_timer *timer = mfb_timer_create();
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		const char *text = "--(2022 DEMO CREW)--";
		int32_t text_x = 0;
		uint32_t text_length = strlen(text);
		uint32_t char_index = 0;
		while (char_index < text_length) {
			char character[] = {0, 0};
			character[0] = (char) r96_next_utf8_code_point(text, &char_index, text_length);
			int32_t text_y = output.height / 2 - font.glyph_width / 2 + (int32_t) (sinf(counter + char_index / 10.0f) * output.height / 4);
			r96_text(&output, &font, character, text_x, text_y, 0xffffffff);
			text_x += font.glyph_width;
		}

		counter += M_1_PI * mfb_timer_delta(timer) * 12;
		mfb_timer_reset(timer);

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
