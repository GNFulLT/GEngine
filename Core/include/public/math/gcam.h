#ifndef GCAM_H
#define GCAM_H

#include "public/math/gmat4.h"
#include "frustum.h"
#include <cassert>

inline gmat4 perspective(float fov, float aspect, float n, float f) {
    float ymax = n * tanf(fov * GPI / 360.0f);
    float xmax = ymax * aspect;
    return frustum(-xmax, xmax, -ymax, ymax, n, f);
}


inline gmat4 ortho(float l, float r, float b, float t,
    float n, float f) {
    if (l == r || t == b || n == f) {
        assert(false);
    }
    return gmat4(
        2.0f / (r - l), 0, 0, 0,
        0, 2.0f / (t - b), 0, 0,
        0, 0, -2.0f / (f - n), 0,
        -((r + l) / (r - l)), -((t + b) / (t - b)), -((f + n) / (f - n)), 1
    );
}

inline gmat4 look_at(const gvec3& position, const gvec3& target,
    const gvec3& up) {
    gvec3 f = (target - position).normalized() * -1.0f;
    gvec3 r = cross(up, f); // Right handed
    if (r == gvec3(0, 0, 0)) {
        assert(false);
    }
    r.normalize();
    gvec3 u = (cross(f, r)).normalized(); // Right handed
    gvec3 t = gvec3(
        -dot(r, position),
        -dot(u, position),
        -dot(f, position)
    );
    return gmat4(
        // Transpose upper 3x3 matrix to invert it
        r.x, u.x, f.x, 0,
        r.y, u.y, f.y, 0,
        r.z, u.z, f.z, 0,
        t.x, t.y, t.z, 1
    );
}


#endif // GCAM