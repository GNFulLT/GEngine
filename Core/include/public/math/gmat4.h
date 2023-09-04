#ifndef GMAT4_H
#define GMAT4_H

#include <math.h>

#include "public/math/gvec4.h"
#include "public/math/gvec3.h"

#define GMAT4_EPSILON 0.000001f

#define M4D(aRow, bCol) \
    a.v[0 * 4 + aRow] * b.v[bCol * 4 + 0] + \
    a.v[1 * 4 + aRow] * b.v[bCol * 4 + 1] + \
    a.v[2 * 4 + aRow] * b.v[bCol * 4 + 2] + \
    a.v[3 * 4 + aRow] * b.v[bCol * 4 + 3]

#define M4V4D(mRow, x, y, z, w) \
    x * m.v[0 * 4 + mRow] + \
    y * m.v[1 * 4 + mRow] + \
    z * m.v[2 * 4 + mRow] + \
    w * m.v[3 * 4 + mRow]

#define M4SWAP(x, y) \
    {float t = x; x = y; y = t; }

#define M4_3X3MINOR(x, c0, c1, c2, r0, r1, r2) \
   (x[c0*4+r0]*(x[c1*4+r1]*x[c2*4+r2]-x[c1*4+r2]* \
   x[c2*4+r1])-x[c1*4+r0]*(x[c0*4+r1]*x[c2*4+r2]- \
   x[c0*4+r2]*x[c2*4+r1])+x[c2*4+r0]*(x[c0*4+r1]* \
   x[c1*4+r2]-x[c0*4+r2]*x[c1*4+r1]))

struct gmat4;

gmat4 operator*(const gmat4& m, float f);

struct gmat4
{
	union
	{
		struct {
			gvec4 right;
			gvec4 up;
			gvec4 forward;
			gvec4 position;
		};

		struct
		{
			//       row 1      row 2       row 3      row 4
			/*col1*/ float xx; float xy; float xz; float xw;
			/*col2*/ float yx; float yy; float yz; float yw;
			/*col3*/ float zx; float zy; float zz; float zw;
			/*col4*/ float tx; float ty; float tz; float tw;
		};
		float v[16];
	};

	inline gmat4() :
		xx(1), xy(0), xz(0), xw(0),
		yx(0), yy(1), yz(0), yw(0),
		zx(0), zy(0), zz(1), zw(0),
		tx(0), ty(0), tz(0), tw(1) {}

	inline gmat4(float* fv) :
		xx(fv[0]), xy(fv[1]), xz(fv[2]), xw(fv[3]),
		yx(fv[4]), yy(fv[5]), yz(fv[6]), yw(fv[7]),
		zx(fv[8]), zy(fv[9]), zz(fv[10]), zw(fv[11]),
		tx(fv[12]), ty(fv[13]), tz(fv[14]), tw(fv[15]) { }

	inline gmat4(
		float _00, float _01, float _02, float _03,
		float _10, float _11, float _12, float _13,
		float _20, float _21, float _22, float _23,
		float _30, float _31, float _32, float _33) :
		xx(_00), xy(_01), xz(_02), xw(_03),
		yx(_10), yy(_11), yz(_12), yw(_13),
		zx(_20), zy(_21), zz(_22), zw(_23),
		tx(_30), ty(_31), tz(_32), tw(_33) { }

	inline gmat4 transposed() const noexcept {
		return gmat4(
			xx, yx, zx, tx,
			xy, yy, zy, ty,
			xz, yz, zz, tz,
			xw, yw, zw, tw
		);
	}

	inline float determinant() const noexcept {
		return v[0] * M4_3X3MINOR(v, 1, 2, 3, 1, 2, 3)
			- v[4] * M4_3X3MINOR(v, 0, 2, 3, 1, 2, 3)
			+ v[8] * M4_3X3MINOR(v, 0, 1, 3, 1, 2, 3)
			- v[12] * M4_3X3MINOR(v, 0, 1, 2, 1, 2, 3);
	}

	inline gmat4 adjugate() const noexcept {
		gmat4 cofactor;
		cofactor.v[0] = M4_3X3MINOR(v, 1, 2, 3, 1, 2, 3);
		cofactor.v[1] = -M4_3X3MINOR(v, 1, 2, 3, 0, 2, 3);
		cofactor.v[2] = M4_3X3MINOR(v, 1, 2, 3, 0, 1, 3);
		cofactor.v[3] = -M4_3X3MINOR(v, 1, 2, 3, 0, 1, 2);
		cofactor.v[4] = -M4_3X3MINOR(v, 0, 2, 3, 1, 2, 3);
		cofactor.v[5] = M4_3X3MINOR(v, 0, 2, 3, 0, 2, 3);
		cofactor.v[6] = -M4_3X3MINOR(v, 0, 2, 3, 0, 1, 3);
		cofactor.v[7] = M4_3X3MINOR(v, 0, 2, 3, 0, 1, 2);
		cofactor.v[8] = M4_3X3MINOR(v, 0, 1, 3, 1, 2, 3);
		cofactor.v[9] = -M4_3X3MINOR(v, 0, 1, 3, 0, 2, 3);
		cofactor.v[10] = M4_3X3MINOR(v, 0, 1, 3, 0, 1, 3);
		cofactor.v[11] = -M4_3X3MINOR(v, 0, 1, 3, 0, 1, 2);
		cofactor.v[12] = -M4_3X3MINOR(v, 0, 1, 2, 1, 2, 3);
		cofactor.v[13] = M4_3X3MINOR(v, 0, 1, 2, 0, 2, 3);
		cofactor.v[14] = -M4_3X3MINOR(v, 0, 1, 2, 0, 1, 3);
		cofactor.v[15] = M4_3X3MINOR(v, 0, 1, 2, 0, 1, 2);
		return cofactor.transposed();
	}

	inline gmat4 inverse() const noexcept {
		float det = determinant();

		if (det == 0.0f) {
			return gmat4();
		}
		gmat4 adj = adjugate();
		return adj * (1.0f / det);
	}
	inline void invert() noexcept{
		float det = determinant();
		if (det == 0.0f) {
			*this = gmat4();
			return;
		}
		*this = adjugate() * (1.0f / det);
	}
};

inline bool operator==(const gmat4& a, const gmat4& b) {
	for (int i = 0; i < 16; ++i) {
		if (fabsf(a.v[i] - b.v[i]) > GMAT4_EPSILON) {
			return false;
		}
	}
	return true;
}
inline bool operator!=(const gmat4& a, const gmat4& b) {
	return !(a == b);
}

inline gmat4 operator+(const gmat4& a, const gmat4& b) {
	return gmat4(
		a.xx + b.xx, a.xy + b.xy, a.xz + b.xz, a.xw + b.xw,
		a.yx + b.yx, a.yy + b.yy, a.yz + b.yz, a.yw + b.yw,
		a.zx + b.zx, a.zy + b.zy, a.zz + b.zz, a.zw + b.zw,
		a.tx + b.tx, a.ty + b.ty, a.tz + b.tz, a.tw + b.tw
	);
}

inline gmat4 operator*(const gmat4& m, float f) {
	return gmat4(
		m.xx * f, m.xy * f, m.xz * f, m.xw * f,
		m.yx * f, m.yy * f, m.yz * f, m.yw * f,
		m.zx * f, m.zy * f, m.zz * f, m.zw * f,
		m.tx * f, m.ty * f, m.tz * f, m.tw * f
	);
}

inline gmat4 operator*(const gmat4& a, const gmat4& b) {
	return gmat4(
		M4D(0, 0), M4D(1, 0), M4D(2, 0), M4D(3, 0),
		M4D(0, 1), M4D(1, 1), M4D(2, 1), M4D(3, 1),
		M4D(0, 2), M4D(1, 2), M4D(2, 2), M4D(3, 2),
		M4D(0, 3), M4D(1, 3), M4D(2, 3), M4D(3, 3) 
	);
}

inline gvec4 operator*(const gmat4& m, const gvec4& v) {
	return gvec4(
		M4V4D(0, v.x, v.y, v.z, v.w),
		M4V4D(1, v.x, v.y, v.z, v.w),
		M4V4D(2, v.x, v.y, v.z, v.w),
		M4V4D(3, v.x, v.y, v.z, v.w)
	);
}

inline gvec3 transform_vector(const gmat4& m, const gvec3& v) {
	return gvec3(
		M4V4D(0, v.x, v.y, v.z, 0.0f),
		M4V4D(1, v.x, v.y, v.z, 0.0f),
		M4V4D(2, v.x, v.y, v.z, 0.0f)
	);
}

inline gvec3 transform_point(const gmat4& m, const gvec3& v) {
	return gvec3(
		M4V4D(0, v.x, v.y, v.z, 1.0f),
		M4V4D(1, v.x, v.y, v.z, 1.0f),
		M4V4D(2, v.x, v.y, v.z, 1.0f)
	);
}

inline gvec3 transform_point(const gmat4& m, const gvec3& v, float& w) {
	float _w = w;
	w = M4V4D(3, v.x, v.y, v.z, _w);
	return gvec3(
		M4V4D(0, v.x, v.y, v.z, _w),
		M4V4D(1, v.x, v.y, v.z, _w),
		M4V4D(2, v.x, v.y, v.z, _w)
	);
}

inline void transpose(gmat4& m) {
	M4SWAP(m.yx, m.xy);
	M4SWAP(m.zx, m.xz);
	M4SWAP(m.tx, m.xw);
	M4SWAP(m.zy, m.yz);
	M4SWAP(m.ty, m.yw);
	M4SWAP(m.tz, m.zw);
}

inline gmat4 translate(gvec3& pos) noexcept
{
	return gmat4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		pos.x, pos.y, pos.z, 1.f
	);
}


#endif // GMAT4_H