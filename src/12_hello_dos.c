#include <MiniFB.h>
#include <stdio.h>
#include "r96/r96.h"
#include "stdlib.h"
#include <math.h>

#define GDB_IMPLEMENTATION
#include "dos/gdbstub.h"

#define num_grunts 100

typedef struct grunt {
	int x, y, vx, vy;
} grunt;

int main(void) {
	gdb_start();
	r96_image image;
	if (!r96_image_init_from_file(&image, "assets/grunt.png")) {
		printf("Couldn't load file 'assets/grunt.png'\n");
		return -1;
	}

	r96_image output;
	r96_image_init(&output, 320, 240);
	struct mfb_window *window = mfb_open("12_hello_dos", output.width, output.height);

	grunt grunts[num_grunts];
	for (int i = 0; i < num_grunts; i++) {
		grunt *grunt = &grunts[i];
		grunt->x = rand() % 320;
		grunt->y = rand() % 200;
		grunt->vx = 1;
		grunt->vy = 1;
	}
	do {
		r96_clear_with_color(&output, 0xff222222);
		for (int i = 0; i < num_grunts; i++) {
			grunt *grunt = &grunts[i];
			if (grunt->x < 0) {
				grunt->x = 0;
				grunt->vx = -grunt->vx;
			}
			if (grunt->x > 320 - 64) {
				grunt->x = 320 - 64;
				grunt->vx = -grunt->vx;
			}
			if (grunt->y < 0) {
				grunt->y = 0;
				grunt->vy = -grunt->vy;
			}
			if (grunt->y > 240 - 64) {
				grunt->y = 240 - 64;
				grunt->vy = -grunt->vy;
			}
			grunt->x += grunt->vx;
			grunt->y += grunt->vy;
			r96_blit_keyed(&output, &image, grunt->x, grunt->y, 0x00000000);
		}
		if (mfb_update_ex(window, output.pixels, output.width, output.height) != STATE_OK) break;
		gdb_checkpoint();
	} while (mfb_wait_sync(window));

	r96_image_dispose(&image);
	r96_image_dispose(&output);
	return 0;
}
