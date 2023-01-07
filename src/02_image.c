#include <MiniFB.h>
#include <stdio.h>
#include "r96/r96.h"

int main(void) {
	r96_image image;
	r96_image_init(&image, 320, 240);
	struct mfb_window *window = mfb_open("02_image", image.width * 2, image.height * 2);
	do {
		for (int i = 0; i < 200; i++) {
			int32_t x = rand() % image.width;
			int32_t y = rand() % image.height;
			uint32_t color = R96_ARGB(255, rand() % 255, rand() % 255, rand() % 255);
			r96_set_pixel(&image, x, y, color);
		}

		if (mfb_get_mouse_button_buffer(window)[MOUSE_BTN_1]) {
			int32_t mouse_x = mfb_get_mouse_x(window);
			int32_t mouse_y = mfb_get_mouse_y(window);
			uint32_t color = r96_get_pixel(&image, mouse_x, mouse_y);
			printf("(%i, %i) = { alpha: %i, red: %i, green: %i, blue: %i }\n", (int)mouse_x, (int)mouse_y, R96_A(color), R96_R(color), R96_G(color), R96_B(color));
		}

		mfb_update_ex(window, image.pixels, image.width, image.height);
	} while (mfb_wait_sync(window));
	r96_image_dispose(&image);
	return 0;
}
