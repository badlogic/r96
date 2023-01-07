#include <MiniFB.h>
#include <stdio.h>
#include "r96/r96.h"
#include <math.h>

void blit(r96_image *dst, r96_image *src, int x, int y) {
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
		int32_t num_pixels = clipped_width;
		while (num_pixels--) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

int main(void) {
	r96_image image;
	if (!r96_image_init_from_file(&image, "assets/grunt.png")) {
		printf("Couldn't load file 'assets/grunt.png'\n");
		return -1;
	}

	r96_image output;
	r96_image_init(&output, 320, 240);
	struct mfb_window *window = mfb_open("09_blit", output.width * 2, output.height * 2);

	do {
		r96_clear_with_color(&output, 0xff222222);
		blit(&output, &image, output.width / 2 - image.width / 2, output.height / 2 - image.height / 2);
		if (mfb_update_ex(window, output.pixels, output.width, output.height) < 0) break;
	} while (mfb_wait_sync(window));

	r96_image_dispose(&image);
	r96_image_dispose(&output);
	return 0;
}
