#include <MiniFB.h>
#include <math.h>
#include <stdio.h>
#include "MiniFB_enums.h"
#include "r96/r96.h"

bool clip_line(float *x1, float *y1, float *x2, float *y2, float min_x, float min_y, float max_x, float max_y) {
	float lx1 = *x1, ly1 = *y1, lx2 = *x2, ly2 = *y2;
	if ((lx1 < min_x && lx2 < min_x) || (lx1 > max_x && lx2 > max_x)) return false;
	if ((ly1 < min_y && ly2 < min_y) || (ly1 > max_y && ly2 > max_y)) return false;

	float x[] = {lx1, lx2};
	float y[] = {ly1, ly2};
	int32_t i = 1;
	do {
		if (x[i] < min_x) {
			x[i] = min_x;
			y[i] = ((ly2 - ly1) / (lx2 - lx1)) * (min_x - lx1) + ly1;
		} else if (x[i] > max_x) {
			x[i] = max_x;
			y[i] = ((ly2 - ly1) / (lx2 - lx1)) * (max_x - lx1) + ly1;
		}
		if (y[i] < min_y) {
			y[i] = min_y;
			x[i] = ((lx2 - lx1) / (ly2 - ly1)) * (min_y - ly1) + lx1;
		} else if (y[i] > max_y) {
			y[i] = max_y;
			x[i] = ((lx2 - lx1) / (ly2 - ly1)) * (max_y - ly1) + lx1;
		}
		i++;
	} while (i <= 1);

	if (!(x[0] < min_x && x[1] < min_x) && !(x[0] > max_x && x[1] > max_x)) {
		*x1 = x[0];
		*y1 = y[0];
		*x2 = x[1];
		*y2 = y[1];
		return true;
	} else {
		return false;
	}
}

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

void line_naive(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = (x2 - x1);
	int32_t delta_y = (y2 - y1);
	int32_t num_pixels_x = abs(delta_x) + 1;
	int32_t num_pixels_y = abs(delta_y) + 1;

	float step_x, step_y;
	uint32_t num_pixels;
	if (num_pixels_x >= num_pixels_y) {
		step_x = delta_x < 0 ? -1 : 1;
		step_y = (float) delta_y / abs(delta_x);
		num_pixels = num_pixels_x;
	} else {
		step_x = (float) delta_x / abs(delta_y);
		step_y = delta_y < 0 ? -1 : 1;
		num_pixels = num_pixels_y;
	}

	float x = x1 + 0.5f, y = y1 + 0.5f;
	for (uint32_t i = 0; i < num_pixels; i++) {
		r96_set_pixel(image, (int32_t) x, (int32_t) y, color);
		x += step_x;
		y += step_y;
	}
}

void line_fixed_point(r96_image *image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	int32_t delta_x = (x2 - x1);
	int32_t delta_y = (y2 - y1);
	int32_t num_pixels_x = abs(delta_x) + 1;
	int32_t num_pixels_y = abs(delta_y) + 1;
	delta_x = int_to_fixed(delta_x, FIXED_8_BITS);
	delta_y = int_to_fixed(delta_y, FIXED_8_BITS);

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
		step_y = fixed_div(delta_y, abs(delta_x), FIXED_8_BITS);
		num_pixels = num_pixels_x;
	} else {
		step_x = fixed_div(delta_x, abs(delta_y), FIXED_8_BITS);
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

void line_sub_pixel(r96_image *image, float x1, float y1, float x2, float y2, float color) {
	if (!clip_line(&x1, &y1, &x2, &y2, 0, 0, image->width, image->height)) return;
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
		step_y = fixed_div(delta_y, abs(delta_x), FIXED_8_BITS);
		num_pixels = fixed_to_int(num_pixels_x, FIXED_8_BITS);
	} else {
		step_x = fixed_div(delta_x, abs(delta_y), FIXED_8_BITS);
		step_y = int_to_fixed(delta_y < 0 ? -1 : 1, FIXED_8_BITS);
		num_pixels = fixed_to_int(num_pixels_y, FIXED_8_BITS);
	}

	int32_t x = x1_fp, y = y1_fp;
	for (uint32_t i = 0; i < num_pixels; i++) {
		int32_t px = fixed_to_int(x, FIXED_8_BITS);
		int32_t py = fixed_to_int(y, FIXED_8_BITS);
		r96_set_pixel(image, px, py, color);
		x += step_x;
		y += step_y;
	}
}

// 6 bits works great for a 320x240 screen and 16-bit integers.
// Larger screens may need to drop the number of bits or use wider integers.
int const BitsOfPrecision = 6;

enum DrawLineDebugMethodEnum {
	DLDME_BresenhamFloat,
	DLDME_BresSliceFloat,
	DLDME_BresSliceFixed,
} DrawLineDebugMethod = DLDME_BresSliceFixed;

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
	struct mfb_window *window = mfb_open("20_line_clipping", window_width * 4, window_height * 4);
	r96_image output;
	r96_image_init(&output, window_width, window_height);
	r96_font font;
	r96_font_init(&font, "assets/tamzen5x9.png", 5, 9);
	do {
		r96_clear_with_color(&output, R96_ARGB(0xff, 0x22, 0x22, 0x22));
		int32_t mouse_x = mfb_get_mouse_x(window) >> 2;
		int32_t mouse_y = mfb_get_mouse_y(window) >> 2;
		r96_hline(&output, 0, 320, 0, 0xffff0000);
		r96_hline(&output, 0, 320, 239, 0xffff0000);
		r96_vline(&output, 0, 0, 239, 0xffff0000);
		r96_vline(&output, 319, 0, 239, 0xffff0000);
		//line_naive(&output, 160, 100, mouse_x, mouse_y, 0xff0000ff);
		line_fixed_point(&output, 160, 100, mouse_x, mouse_y, 0xff00ff00);
		line_sub_pixel(&output, 160 + 0.5f, 100 + 0.5f, mouse_x + 0.5f, mouse_y + 0.5f, 0xffff0000);
		//line_sub_pixel(&output, 160, 100, 261, -3, 0xff00ff00);
		// line_sub_pixel(&output, 160, 100, 261, 0, 0xff00ff00);

		float x1 = 33.6566939887855, y1 = 31.5579464410236;
		float x2 = 65.9397151539753, y2 = 4.99648529337555;
		line_tom(&output, x1, y1, x2, y2, 0xffff0000);

		char text[300];
		sprintf(text, "%i %i", mouse_x, mouse_y);
		r96_text(&output, &font, text, 0, 0, 0xffffffff);

		if (mfb_update_ex(window, output.pixels, window_width, window_height) != STATE_OK) break;
	} while (mfb_wait_sync(window));
	return 0;
}
