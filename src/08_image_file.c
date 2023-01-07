#include <stdio.h>
#include "r96/r96.h"

int main(void) {
	r96_image image;
	if (!r96_image_init_from_file(&image, "assets/grunt.png")) {
		printf("Couldn't load file 'assets/grunt.png'\n");
		return -1;
	}
	printf("Loaded file 'assets/grunt.png', %ix%i pixels\n", image.width, image.height);
	r96_image_dispose(&image);
	return 0;
}
