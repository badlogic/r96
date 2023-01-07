#include <MiniFB.h>
#include <stdlib.h>

int main(void) {
	const int res_x = 320, res_y = 240;
	struct mfb_window *window = mfb_open("00_basic_window", res_x, res_y);
	uint32_t *pixels = (uint32_t *) malloc(sizeof(uint32_t) * res_x * res_y);
	do {
		mfb_update_ex(window, pixels, res_x, res_y);
	} while (mfb_wait_sync(window));
	return 0;
}
