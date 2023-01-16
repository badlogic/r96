#include <MiniFB.h>
#include <math.h>
#include <stdio.h>
#include "r96/r96.h"
#include <assert.h>

#define FIXED_8_BITS 8
#define FIXED_8_ONE (1 << FIXED_8_BITS)
#define FIXED_8_ZERO_POINT_FIVE (1 << (FIXED_8_BITS - 1))

static inline int32_t float_to_fixed(float v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

static inline int32_t int_to_fixed(int32_t v, int32_t bits) {
	return (int32_t) (v * (1 << bits));
}

static inline int32_t fixed_to_int(int32_t v, int32_t bits) {
	return v >> bits;
}

static inline int32_t fixed_div(int32_t a, int32_t b, int32_t bits) {
	return ((int64_t) a * (1 << bits)) / b;
}

static inline int32_t fixed_floor(int32_t v, int32_t bits) {
	int32_t fpOne = (1 << bits);
	int32_t subMask = fpOne - 1;
	return (v & ~subMask);
}

void line_naive_float(r96_image *image, float x1, float y1, float x2, float y2, uint32_t color) {
	float delta_x = (x2 - x1);
	float delta_y = (y2 - y1);
	uint32_t num_pixels_x = floorf(delta_x) + 1;
	uint32_t num_pixels_y = floorf(delta_y) + 1;

	float step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = delta_x < 0 ? -1 : 1;
		step_y = delta_y / fabs(delta_x);
		num_pixels = num_pixels_x;
	} else {
		step_x = delta_x / fabs(delta_y);
		step_y = delta_y < 0 ? -1 : 1;
		num_pixels = num_pixels_y;
	}

	float x = x1 + 0.5f, y = y1 + 0.5f;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_set_pixel(image, roundf(x), roundf(y), color);
		x += step_x;
		y += step_y;
	}
}

void line_fixed_point(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = (x2 - x1);
	int32_t delta_y = (y2 - y1);
	int32_t num_pixels_x = abs(delta_x);
	int32_t num_pixels_y = abs(delta_y);

	if (delta_x == 0) {
		r96_vline(image, x1, y1, y2, color);
		return;
	}
	if (delta_y == 0) {
		r96_hline(image, x1, x2, y1, color);
		return;
	}

	int32_t step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = int_to_fixed(delta_x < 0 ? -1 : 1, FIXED_8_BITS);
		step_y = float_to_fixed((float) delta_y / num_pixels_x, FIXED_8_BITS);
		num_pixels = num_pixels_x;
	} else {
		step_x = float_to_fixed((float) delta_x / num_pixels_y, FIXED_8_BITS);
		step_y = int_to_fixed(delta_y < 0 ? -1 : 1, FIXED_8_BITS);
		num_pixels = num_pixels_y;
	}

	int32_t x = int_to_fixed(x1, FIXED_8_BITS) + FIXED_8_ZERO_POINT_FIVE, y = int_to_fixed(y1, FIXED_8_BITS) + FIXED_8_ZERO_POINT_FIVE;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_set_pixel(image, fixed_to_int(x, FIXED_8_BITS), fixed_to_int(y, FIXED_8_BITS), color);
		x += step_x;
		y += step_y;
	}
}

void line_sub_pixel(r96_image *image, float x1, float y1, float x2, float y2, uint32_t color) {
	int32_t x1_fp = float_to_fixed(x1, FIXED_8_BITS);
	int32_t y1_fp = float_to_fixed(y1, FIXED_8_BITS);
	int32_t x2_fp = float_to_fixed(x2, FIXED_8_BITS);
	int32_t y2_fp = float_to_fixed(y2, FIXED_8_BITS);
	int32_t delta_x = (x2_fp - x1_fp);
	int32_t delta_y = (y2_fp - y1_fp);
	int32_t num_pixels_x = fixed_floor(abs(delta_x), FIXED_8_BITS) + FIXED_8_ONE;
	int32_t num_pixels_y = fixed_floor(abs(delta_y), FIXED_8_BITS) + FIXED_8_ONE;

	if (delta_x == 0) {
		r96_vline(image, (int) x1, (int) y1, (int) y2, color);
		return;
	}
	if (delta_y == 0) {
		r96_hline(image, (int) x1, (int) x2, (int) y1, color);
		return;
	}

	int32_t step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = int_to_fixed(delta_x < 0 ? -1 : 1, FIXED_8_BITS);
		step_y = fixed_div(delta_y, num_pixels_x, FIXED_8_BITS);
		num_pixels = fixed_to_int(num_pixels_x, FIXED_8_BITS);
	} else {
		step_x = fixed_div(delta_x, num_pixels_y, FIXED_8_BITS);
		step_y = int_to_fixed(delta_y < 0 ? -1 : 1, FIXED_8_BITS);
		num_pixels = fixed_to_int(num_pixels_y, FIXED_8_BITS);
	}

	int32_t x = x1_fp, y = y1_fp;
	// int32_t last_px = -1, last_py = -1;
	for (uint32_t i = 0; i < num_pixels; i++) {
		int32_t px = fixed_to_int(x, FIXED_8_BITS);
		int32_t py = fixed_to_int(y, FIXED_8_BITS);
		// assert(last_px != px && last_py != py);
		r96_set_pixel(image, px, py, color);
		printf("%i %i\n", px, py);
		// last_px = px;
		// last_py = py;
		x += step_x;
		y += step_y;
	}
}

// 6 bits works great for a 320x240 screen and 16-bit integers.
// Larger screens may need to drop the number of bits or use wider integers.
int const BitsOfPrecision = 8;

enum DrawLineDebugMethodEnum {
	DLDME_BresenhamFloat,
	DLDME_BresSliceFloat,
	DLDME_BresSliceFixed,
} DrawLineDebugMethod = DLDME_BresSliceFloat;

void line_tom(r96_image *image, float f_X1, float f_Y1, float f_X2, float f_Y2, uint32_t Colour) {
	int X1 = float_to_fixed(f_X1, BitsOfPrecision);
	int Y1 = float_to_fixed(f_Y1, BitsOfPrecision);
	int X2 = float_to_fixed(f_X2, BitsOfPrecision);
	int Y2 = float_to_fixed(f_Y2, BitsOfPrecision);
	int const PrecisionMask = (1 << BitsOfPrecision) - 1;
	int const FramebufferStride = image->width;

	int DeltaX = X2 - X1;
	int DeltaY = Y2 - Y1;
	int FracX = X1 & PrecisionMask;
	int FracY = Y1 & PrecisionMask;
	int X1W = X1 >> BitsOfPrecision;
	int Y1W = Y1 >> BitsOfPrecision;
	int PixelsToGoX = (X2 >> BitsOfPrecision) - (X1W);
	int PixelsToGoY = (Y2 >> BitsOfPrecision) - (Y1W);

	int DeltaB;
	int DeltaS;
	int FracB;
	int FracS;
	int PixelStepB;// = 1;
	int PixelStepS;// = FramebufferStride;
	int PixelsToGoB;
	int PixelsToGoS;

	if (DeltaX >= 0) {
		if (DeltaY >= 0) {
			DeltaB = DeltaX;
			DeltaS = DeltaY;
			FracB = FracX;
			FracS = FracY;
			PixelsToGoB = PixelsToGoX;
			PixelsToGoS = PixelsToGoY;
			PixelStepB = 1;
			PixelStepS = FramebufferStride;
		} else {
			DeltaB = DeltaX;
			DeltaS = -DeltaY;
			FracB = FracX;
			FracS = PrecisionMask - FracY;
			PixelsToGoB = PixelsToGoX;
			PixelsToGoS = -PixelsToGoY;
			PixelStepB = 1;
			PixelStepS = -FramebufferStride;
		}
	} else {
		if (DeltaY >= 0) {
			DeltaB = -DeltaX;
			DeltaS = DeltaY;
			FracB = PrecisionMask - FracX;
			FracS = FracY;
			PixelsToGoB = -PixelsToGoX;
			PixelsToGoS = PixelsToGoY;
			PixelStepB = -1;
			PixelStepS = FramebufferStride;
		} else {
			DeltaB = -DeltaX;
			DeltaS = -DeltaY;
			FracB = PrecisionMask - FracX;
			FracS = PrecisionMask - FracY;
			PixelsToGoB = -PixelsToGoX;
			PixelsToGoS = -PixelsToGoY;
			PixelStepB = -1;
			PixelStepS = -FramebufferStride;
		}
	}

	if (DeltaB < DeltaS) {
		int Temp;
		Temp = DeltaB;
		DeltaB = DeltaS;
		DeltaS = Temp;
		Temp = FracB;
		FracB = FracS;
		FracS = Temp;
		Temp = PixelsToGoB;
		PixelsToGoB = PixelsToGoS;
		PixelsToGoS = Temp;
		Temp = PixelStepB;
		PixelStepB = PixelStepS;
		PixelStepS = Temp;
	}

	if (PixelsToGoB > 0) {
		if (DrawLineDebugMethod == DLDME_BresenhamFloat) {
			int PixelsToGo = PixelsToGoB;
			float const PrecisionF = (float) (1 << BitsOfPrecision);
			float DeltaBf = (float) DeltaB / PrecisionF;
			float DeltaSf = (float) DeltaS / PrecisionF;
			float FracBf = (float) FracB / PrecisionF;
			float FracSf = (float) FracS / PrecisionF;

			float Distf = ((FracSf - 0.5f) * DeltaBf) - ((FracBf - 0.5f) * DeltaSf);
			uint32_t *CurPixAddr = image->pixels + X1W + Y1W * image->width;

			while (PixelsToGo > 0) {
				if (Distf > (DeltaBf * 0.5f)) {
					CurPixAddr += PixelStepS;
					Distf -= DeltaBf;
				}
				*CurPixAddr = Colour;
				CurPixAddr += PixelStepB;
				Distf += DeltaSf;
				PixelsToGo--;
			}
		} else if (DrawLineDebugMethod == DLDME_BresSliceFloat) {
			float const PrecisionF = (float) (1 << BitsOfPrecision);

			int PixelsToGo = PixelsToGoB;
			float DeltaBf = (float) DeltaB / PrecisionF;
			float DeltaSf = (float) DeltaS / PrecisionF;
			float FracBf = (float) FracB / PrecisionF;
			float FracSf = (float) FracS / PrecisionF;
			int BigStep = (int) floor(DeltaBf);
			if (DeltaSf > 0.0f) {
				BigStep = (int) floor(DeltaBf / DeltaSf);
			}

			float Distf = (DeltaBf * (FracSf - 0.5f)) - (DeltaSf * (FracBf - 0.5f));
			Distf -= DeltaBf * 0.5f;

			uint32_t *CurPixAddr = image->pixels + X1W + Y1W * image->width;

			int FirstStep = 0;
			while ((Distf <= 0.0f) && (PixelsToGo > 0)) {
				FirstStep++;
				Distf += DeltaSf;
				PixelsToGo--;
			}
			for (int i = 0; i < FirstStep; i++) {
				*CurPixAddr = Colour;
				CurPixAddr += PixelStepB;
			}
			Distf -= DeltaBf;
			CurPixAddr += PixelStepS;

			if (PixelsToGo > 0) {
				float StepDistf = DeltaSf * (float) BigStep;

				while (true) {
					int StepSize = BigStep;
					Distf += StepDistf;
					if (Distf > 0.0f) {
						StepSize = BigStep;
					} else {
						StepSize = BigStep + 1;
						Distf += DeltaSf;
					}
					Distf -= DeltaBf;

					if (PixelsToGo <= StepSize) {
						break;
					}
					PixelsToGo -= StepSize;
					for (int i = 0; i < StepSize; i++) {
						*CurPixAddr = Colour;
						CurPixAddr += PixelStepB;
					}
					CurPixAddr += PixelStepS;
				}

				for (int i = 0; i < PixelsToGo; i++) {
					*CurPixAddr = Colour;
					CurPixAddr += PixelStepB;
				}
			}
		} else if (DrawLineDebugMethod == DLDME_BresSliceFixed) {
			int PixelsToGo = PixelsToGoB;
			uint32_t *CurPixAddr = image->pixels + X1W + Y1W * image->width;

			if (PixelsToGoS == 0) {
				for (int i = 0; i < PixelsToGo; i++) {
					*CurPixAddr = Colour;
					CurPixAddr += PixelStepB;
				}
			} else {
				int const LocalBitsOfPrecision = (BitsOfPrecision == 0) ? 1 : BitsOfPrecision;
				int const PrecisionOne = (1 << LocalBitsOfPrecision);
				int const PrecisionHalf = (PrecisionOne >> 1);
				int BigStep = DeltaB >> LocalBitsOfPrecision;
				int StepDist = DeltaB;
				if (DeltaS > 0) {
					BigStep = DeltaB / DeltaS;
					StepDist = BigStep * DeltaS;
				}
#if 0
                // Also remember that "Dist" is the size of TWO chunks of "Precision" bits
                int Dist = ( DeltaB * ( FracS - PrecisionHalf ) ) - ( DeltaS * ( FracB - PrecisionHalf ) );
                // Now offset by 0.5*DeltaX as described above, so we can just test against zero.
                Dist -= ( DeltaB << ( LocalBitsOfPrecision - 1 ) );
#else
				int Dist = (DeltaB * (FracS - PrecisionOne)) - (DeltaS * (FracB - PrecisionHalf));
#endif
				int DeltaBPrec = DeltaB << LocalBitsOfPrecision;
				int DeltaSPrec = DeltaS << LocalBitsOfPrecision;
				int StepDistPrec = StepDist << LocalBitsOfPrecision;

				int FirstStep = 0;
				while ((Dist <= 0) && (PixelsToGo > 0)) {
					FirstStep++;
					Dist += DeltaSPrec;
					PixelsToGo--;
				}

				for (int i = 0; i < FirstStep; i++) {
					*CurPixAddr = Colour;
					CurPixAddr += PixelStepB;
				}
				CurPixAddr += PixelStepS;

				if (PixelsToGo > 0) {
					Dist -= DeltaBPrec;

					int StepDistMinusDeltaBPrec = StepDistPrec - DeltaBPrec;
					Dist += DeltaBPrec;
					while (true) {
						int StepSize = BigStep;
						Dist += StepDistMinusDeltaBPrec;
						if (Dist <= 0) {
							StepSize = BigStep + 1;
							Dist += DeltaSPrec;
						}

						int PixelsToGoBNew = PixelsToGo - StepSize;
						if (PixelsToGoBNew <= 0) {
							break;
						}
						PixelsToGo = PixelsToGoBNew;
						for (int i = 0; i < StepSize; i++) {
							*CurPixAddr = Colour;
							CurPixAddr += PixelStepB;
						}
						CurPixAddr += PixelStepS;
					}

					for (int i = 0; i < PixelsToGo; i++) {
						*CurPixAddr = Colour;
						CurPixAddr += PixelStepB;
					}
				}
			}
		}
	}
}

int main(void) {
	const int window_width = 320, window_height = 240;
	struct mfb_window *window = mfb_open("19_sub_pixel_line", window_width * 4, window_height * 4);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/tamzen5x9.png", 5, 9);
	struct mfb_timer *timer = mfb_timer_create();
	float delta = M_PI / 4, a = 4, b = 6;
	do {
		delta += mfb_timer_delta(timer) / 40;
		mfb_timer_reset(timer);

		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));

		// delta = 2.48f;

		float last_x = 0, last_y = 0;
		int segments = 50;
		srand(0);
		for (float i = 0; i <= segments; i++) {
			float theta = i / (float) segments * M_PI;
			float x = 100 * sinf(a * theta + delta) / 2 + window_width / 2.0;
			float y = 100 * sinf(b * theta) / 2 + window_height / 2.0;
			// uint32_t color = R96_ARGB(0xff, rand() % 255, rand() % 255, rand() % 255);
			if (i > 0) {
				line_fixed_point(&output, (int) last_x - 100, (int) last_y, (int) x - 100, (int) y, 0xffff0000);
				line_sub_pixel(&output, last_x, last_y, x, y, 0xff00ff00);
				line_tom(&output, last_x + 100, last_y, x + 100, y, 0xffff00ff);
			}
			last_x = x;
			last_y = y;
		}

		char text[100];
		sprintf(text, "delta: %f", delta);
		r96_text(&output, &font, text, 0, 0, 0xffffffff);
		printf("delta %f\n", delta);

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
