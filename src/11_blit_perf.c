#include <MiniFB.h>
#include <stdio.h>
#include "MiniFB_enums.h"
#include "r96/r96.h"
#include <math.h>
#include <string.h>

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
		for (int i = 0; i < clipped_width; i++) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void blit_keyed(r96_image *dst, r96_image *src, int x, int y, uint32_t color_key) {
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
			uint32_t color = *src_pixel;
			src_pixel++;
			if (color == color_key) {
				dst_pixel++;
				continue;
			}
			*dst_pixel++ = color;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void blit_keyed_opt1(r96_image *dst, r96_image *src, int x, int y, uint32_t color_key) {
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
			uint32_t color = *src_pixel;
			if (color != color_key) {
				*dst_pixel = color;
			}
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void blit_keyed_opt2(r96_image *dst, r96_image *src, int x, int y, uint32_t color_key) {
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
		for (int i = 0; i < clipped_width; i++) {
			uint32_t color = *src_pixel;
			if (color != color_key) {
				*dst_pixel = color;
			}
			src_pixel++;
			dst_pixel++;
		}
		dst_pixel += dst_next_row;
		src_pixel += src_next_row;
	}
}

void blit_keyed_opt3(r96_image *dst, r96_image *src, int x, int y, uint32_t color_key) {
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
		for (int i = 0; i < clipped_width; i++) {
			uint32_t color = src_pixel[i];
			if (color != color_key) {
				dst_pixel[i] = color;
			}
		}
		dst_pixel += dst_next_row + clipped_width;
		src_pixel += src_next_row + clipped_width;
	}
}

void blit_keyed_opt4(r96_image *dst, r96_image *src, int x, int y, uint32_t color_key) {
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
		for (int i = 0; i < clipped_width; i++) {
			uint32_t src_color = *src_pixel;
			uint32_t dst_color = *dst_pixel;
			*dst_pixel = src_color != color_key ? src_color : dst_color;
			src_pixel++;
			dst_pixel++;
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
	struct mfb_window *window = mfb_open("11_blit_perf", output.width * 3, output.height * 3);
	struct mfb_timer *timer = mfb_timer_create();
	do {
		r96_clear_with_color(&output, 0xff222222);

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			r96_rect(&output, rand() % output.width, rand() % output.height, 64, 64, 0xffffffff);
		}
		printf("rect()                  %f\n", mfb_timer_delta(timer));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			blit(&output, &image, rand() % output.width, rand() % output.height);
		}
		printf("blit()                  %f\n", mfb_timer_delta(timer));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			blit_keyed(&output, &image, rand() % output.width, rand() % output.height, 0x0);
		}
		printf("blit_keyed()            %f\n", mfb_timer_delta(timer));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			blit_keyed_opt1(&output, &image, rand() % output.width, rand() % output.height, 0x0);
		}
		printf("blit_keyed_opt1()       %f\n", mfb_timer_delta(timer));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			blit_keyed_opt2(&output, &image, rand() % output.width, rand() % output.height, 0x0);
		}
		printf("blit_keyed_opt2()       %f\n", mfb_timer_delta(timer));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			blit_keyed_opt3(&output, &image, rand() % output.width, rand() % output.height, 0x0);
		}
		printf("blit_keyed_opt3()       %f\n", mfb_timer_delta(timer));

		srand(0);
		mfb_timer_reset(timer);
		for (int i = 0; i < 20000; i++) {
			blit_keyed_opt4(&output, &image, rand() % output.width, rand() % output.height, 0x0);
		}
		printf("blit_keyed_opt4()       %f\n", mfb_timer_delta(timer));

		if (mfb_update_ex(window, output.pixels, output.width, output.height) < 0) break;
	} while (mfb_wait_sync(window));

	mfb_timer_destroy(timer);
	r96_image_dispose(&image);
	r96_image_dispose(&output);
	return 0;
}
