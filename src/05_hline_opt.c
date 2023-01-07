#include <MiniFB.h>
#include <stdio.h>
#include "r96/r96.h"

void hline(r96_image *image, int32_t x1, int32_t x2, int32_t y, uint32_t color) {
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

int main(void) {
	r96_image image;
	r96_image_init(&image, 320, 240);
	struct mfb_window *window = mfb_open("05_hline_opt", image.width * 2, image.height * 2);
	struct mfb_timer *timer = mfb_timer_create();
	do {
		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 200000; i++) {
			uint32_t color = R96_ARGB(255, rand() % 255, rand() % 255, rand() % 255);
			hline(&image, rand() % image.width, rand() % image.width, rand() % image.height, color);
		}
		printf("Took: %f\n", mfb_timer_delta(timer));

		if (mfb_update_ex(window, image.pixels, image.width, image.height) < 0) break;
	} while (mfb_wait_sync(window));
	r96_image_dispose(&image);
	return 0;
}
