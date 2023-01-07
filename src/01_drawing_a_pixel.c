#include <MiniFB.h>
#include <stdlib.h>
#include <string.h>
#include "r96/r96.h"

int main(void) {
	const int res_x = 320, res_y = 240;
	struct mfb_window *window = mfb_open("01_drawing_a_pixel", res_x, res_y);
	uint32_t *pixels = (uint32_t *) malloc(sizeof(uint32_t) * res_x * res_y);
	do {
		for (int i = 0; i < 200; i++) {
			int32_t x = rand() % res_x;
			int32_t y = rand() % res_y;
			uint32_t color = R96_ARGB(255, rand() % 255, rand() % 255, rand() % 255);
			pixels[x + y * res_x] = color;
		}

		if (mfb_get_mouse_button_buffer(window)[MOUSE_BTN_1]) {
			memset(pixels, 0, sizeof(uint32_t) * res_x * res_y);
		}

		mfb_update_ex(window, pixels, res_x, res_y);
	} while (mfb_wait_sync(window));
	return 0;
}
