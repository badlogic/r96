#include <MiniFB.h>
#include <stdio.h>
#include "r96/r96.h"

void rect(r96_image *image, int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t color) {
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

	for (int y = y1; y <= y2; y++)
		r96_hline(image, x1, x2, y, color);
}

int main(void) {
	r96_image image;
	r96_image_init(&image, 320, 240);
	struct mfb_window *window = mfb_open("06_rect", image.width * 2, image.height * 2);
	struct mfb_timer *timer = mfb_timer_create();
	do {
		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 200000; i++) {
			uint32_t color = R96_ARGB(255, rand() % 255, rand() % 255, rand() % 255);
			rect(&image, rand() % image.width, rand() % image.width, rand() % (image.width / 5), rand() % (image.height / 5), color);
		}
		printf("Took: %f\n", mfb_timer_delta(timer));

		if (mfb_update_ex(window, image.pixels, image.width, image.height) < 0) break;
	} while (mfb_wait_sync(window));
	r96_image_dispose(&image);
	return 0;
}
