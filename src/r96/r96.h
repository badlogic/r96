#pragma once

#include <stdint.h>

#define R96_ARGB(alpha, red, green, blue) (uint32_t)(((uint8_t) (alpha) << 24) | ((uint8_t) (red) << 16) | ((uint8_t) (green) << 8) | (uint8_t) (blue))
