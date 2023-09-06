#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "public/math/gmat4.h"

// http://www.songho.ca/opengl/gl_projectionmatrix.html
inline gmat4 frustum(float l, float r, float b,
    float t, float n, float f) {
    if (l == r || t == b || n == f) {
        return gmat4(); 
    }
    return gmat4(
        (2.0f * n) / (r - l), 0, 0, 0,
        0, (2.0f * n) / (t - b), 0, 0,
        (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
        0, 0, (-2 * f * n) / (f - n), 0
    );
}


#endif // FRUSTUM_H