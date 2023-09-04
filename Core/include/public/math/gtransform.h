#ifndef GTRANSFORM_H
#define GTRANSFORM_H

#include "public/math/gquat.h"

// https://research.cs.wisc.edu/graphics/Courses/838-s2002/Papers/polar-decomp.pdf
struct gtransform {

    gvec3 position;
    gquat rotation;
    gvec3 scale;
    
    gtransform() : position(0, 0, 0), rotation(0, 0, 0, 1), scale(1, 1, 1) {}
    
    gtransform(const gvec3& _pos, const gquat& _rot, const gvec3& _scale) : position(_pos), rotation(_rot), scale(_scale) {}

    inline void combine_with(const gtransform& b) noexcept
    {
        scale = scale * b.scale;
        rotation = rotation * b.rotation * rotation;
        position = position * position * (scale * b.position);
    }

    inline gtransform inversed() const noexcept
    {
        gtransform inv;
        inv.rotation = (rotation).inverse();
        inv.scale.x = fabs(scale.x) < GVEC3_EPSILON ?
            0.0f : 1.0f / scale.x;
        inv.scale.y = fabs(scale.y) < GVEC3_EPSILON ?
            0.0f : 1.0f / scale.y;
        inv.scale.z = fabs(scale.z) < GVEC3_EPSILON ?
            0.0f : 1.0f / scale.z;
        gvec3 invTrans = position * -1.0f;
        inv.position = inv.rotation * (inv.scale * invTrans);
        return inv;
    }

    inline gmat4 to_mat4() const noexcept {
        // First, extract the rotation basis of the transform
        gvec3 x = rotation * gvec3(1, 0, 0);
        gvec3 y = rotation * gvec3(0, 1, 0);
        gvec3 z = rotation * gvec3(0, 0, 1);

        x = x * scale.x;
        y = y * scale.y;
        z = z * scale.z;

        // Extract the position of the transform
        return gmat4(
            x.x, x.y, x.z, 0, // X basis (& Scale)
            y.x, y.y, y.z, 0, // Y basis (& scale)
            z.x, z.y, z.z, 0, // Z basis (& scale)
            position.x, position.y, position.z, 1  // Position
        );
    }
};

inline gtransform mat4_to_transform(const gmat4& m) {
    gtransform out;
    out.position = gvec3(m.v[12], m.v[13], m.v[14]);
    out.rotation = mat4_to_quat(m);
    gmat4 rotScaleMat(
        m.v[0], m.v[1], m.v[2], 0,
        m.v[4], m.v[5], m.v[6], 0,
        m.v[8], m.v[9], m.v[10], 0,
        0, 0, 0, 1
    );
    gmat4 invRotMat = (out.rotation).inverse().to_mat4();
    gmat4 scaleSkewMat = rotScaleMat * invRotMat;
    out.scale = gvec3(
        scaleSkewMat.v[0],
        scaleSkewMat.v[5],
        scaleSkewMat.v[10]
    );
    return out;
}

inline gvec3 transformVector(const gtransform& a, const gvec3& b) {
    gvec3 out;
    out = a.rotation * (a.scale * b);
    return out;
}

inline gvec3 transform_point(const gtransform& a, const gvec3& b) {
    gvec3 out;
    out = a.rotation * (a.scale * b);
    out = a.position + out;
    return out;
}

inline gtransform mix(const gtransform& a, const gtransform& b, float t) {
    gquat bRot = b.rotation;
    if (dot(a.rotation, bRot) < 0.0f) {
        bRot = -bRot;
    }
    return gtransform(
        lerp(a.position, b.position, t),
        nlerp(a.rotation, bRot, t),
        lerp(a.scale, b.scale, t));
}
#endif // GTRANSFORM_H