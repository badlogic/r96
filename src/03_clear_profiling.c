#include <MiniFB.h>
#include <stdio.h>
#include "r96/r96.h"

int main(void) {
	const int num_iterations = 200000;
	r96_image image;
	r96_image_init(&image, 320, 240);

	struct mfb_timer *timer = mfb_timer_create();
	mfb_timer_reset(timer);
	for (int i = 0; i < num_iterations; i++)
		r96_clear(&image);
	printf("r96_clear()                 %f secs\n", mfb_timer_delta(timer));

	mfb_timer_reset(timer);
	for (int i = 0; i < num_iterations; i++)
		r96_clear_with_color(&image, 0x0);
	printf("r96_clear_with_color()      %f secs\n", mfb_timer_delta(timer));

	r96_image_dispose(&image);
	return 0;
}
