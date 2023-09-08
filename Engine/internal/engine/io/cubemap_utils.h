#ifndef CUBEMAP_UTILS_H
#define CUBEMAP_UTILS_H


#include "internal/engine/io/bitmap.h"

Bitmap convertEquirectangularMapToVerticalCross(const Bitmap& b);
Bitmap convertVerticalCrossToCubeMapFaces(const Bitmap& b);

template <typename T>
inline T clamp(T v, T a, T b)
{
	if (v < a) return a;
	if (v > b) return b;
	return v;
}

#endif // CUBEMAP_UTILS_H