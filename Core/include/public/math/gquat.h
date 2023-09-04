#ifndef GQUAT_H
#define GQUAT_H

#include "public/math/gmat4.h"

#define GQUAT_EPSILON 0.000001f
struct gquat;

gvec3 operator*(const gquat& q, const gvec3& v);
float dot(const gquat& a, const gquat& b);
gquat nlerp(const gquat& from, const gquat& to, float t);

struct gquat
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		struct
		{
			gvec3 vector;
			float scalar;
		};
		float v[4];
	};

	inline gquat() :
		x(0), y(0), z(0), w(1) { }
	inline gquat(float _x, float _y, float _z, float _w)
		: x(_x), y(_y), z(_z), w(_w) {}


	inline gvec3 get_axis() const noexcept
	{
		return vector.normalized();
	}

	inline float get_angle() const noexcept
	{
		return 2.f * acosf(w);
	}
	
	inline float len_sq() const noexcept {
		return x * x + y * y + z * z + w * w;
	}
	
	inline float len() const noexcept{
		float lenSq = x * x + y * y + z * z + w * w;
		if (lenSq < GQUAT_EPSILON) {
			return 0.0f;
		}
		return sqrtf(lenSq);
	}

	inline void normalize() noexcept {
		float lenSq = x * x + y * y + z * z + w * w;
		if (lenSq < GQUAT_EPSILON) {
			return;
		}
		float i_len = 1.0f / sqrtf(lenSq);
		x *= i_len;
		y *= i_len;
		z *= i_len;
		w *= i_len;
	}

	inline gquat normalized() const noexcept {
		float lenSq = x * x + y * y + z * z + w * w;
		if (lenSq < GQUAT_EPSILON) {
			return gquat();
		}
		float il = 1.0f / sqrtf(lenSq);
		return gquat(x * il, y * il, z * il, w * il);
	}

	inline gquat conjugate() const noexcept{
		return gquat(
			-x,
			-y,
			-z,
			w
		);
	}
	
	inline gquat inverse() const noexcept {
		float lenSq = x * x + y * y + z * z + w * w;
		if (lenSq < GQUAT_EPSILON) {
			return gquat();
		}
		float recip = 1.0f / lenSq;
		return gquat(-x * recip,
			-y * recip,
			-z * recip,
			w * recip
		);
	}

	inline gmat4 to_mat4() const noexcept
	{
		gvec3 r = *this * gvec3(1, 0, 0);
		gvec3 u = *this * gvec3(0, 1, 0);
		gvec3 f = *this * gvec3(0, 0, 1);
		return gmat4(r.x, r.y, r.z, 0,
			u.x, u.y, u.z, 0,
			f.x, f.y, f.z, 0,
			0, 0, 0, 1
		);
	}
};




inline gquat operator+(const gquat& a, const gquat& b) {
	return gquat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline gquat operator-(const gquat& a, const gquat& b) {
	return gquat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline gquat operator*(const gquat& a, float b) {
	return gquat(a.x * b, a.y * b, a.z * b, a.w * b);
}
inline gquat operator-(const gquat& q) {
	return gquat(-q.x, -q.y, -q.z, -q.w);
}


inline gvec3 operator*(const gquat& q, const gvec3& v) {
	return q.vector * 2.0f * dot(q.vector, v) +
		v * (q.scalar * q.scalar - dot(q.vector, q.vector)) +
		cross(q.vector, v) * 2.0f * q.scalar;
}

inline gquat operator*(const gquat& Q1, const gquat& Q2) noexcept {
	gquat result;
	result.scalar = Q2.scalar * Q1.scalar -
		dot(Q2.vector, Q1.vector);
	result.vector = (Q1.vector * Q2.scalar) +
		(Q2.vector * Q1.scalar) + cross(Q2.vector, Q1.vector);
	return result;
}


inline bool operator==(const gquat& left, const gquat& right) {
	return (fabsf(left.x - right.x) <= GQUAT_EPSILON &&
		fabsf(left.y - right.y) <= GQUAT_EPSILON &&
		fabsf(left.z - right.z) <= GQUAT_EPSILON &&
		fabsf(left.w - right.w) <= GQUAT_EPSILON);
}
inline bool operator!=(const gquat& a, const gquat& b) {
	return !(a == b);
}

inline gquat operator^(const gquat& q, float f) {
	float angle = 2.0f * acosf(q.scalar);
	gvec3 axis = (q.vector).normalized();
	float halfCos = cosf(f * angle * 0.5f);
	float halfSin = sinf(f * angle * 0.5f);
	return gquat(axis.x * halfSin,
		axis.y * halfSin,
		axis.z * halfSin,
		halfCos
	);
}


inline float dot(const gquat& a, const gquat& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline gquat from_to(const gvec3& from, const gvec3& to) noexcept {
	auto f = from.normalized();
	auto t = to.normalized();
	if (f == t)
		return gquat();
	else if (f == t * -1.0f) {
		gvec3 ortho = gvec3(1, 0, 0);
		if (fabsf(f.y) < fabsf(f.x)) {
			ortho = gvec3(0, 1, 0);
		}
		if (fabsf(f.z) < fabs(f.y) && fabs(f.z) < fabsf(f.x)) {
			ortho = gvec3(0, 0, 1);
		}
		gvec3 axis = (cross(f, ortho)).normalized();
		return gquat(axis.x, axis.y, axis.z, 0);
	}
	gvec3 half = (f + t).normalized();
	gvec3 axis = cross(f, half);
	return gquat(axis.x, axis.y, axis.z, dot(f, half));
}

inline gquat nlerp(const gquat& from, const gquat& to, float t) {
	return (from + (to - from) * t).normalized();
}
inline gquat slerp(const gquat& start, const gquat& end, float t) {
	if (fabsf(dot(start, end)) > 1.0f - GQUAT_EPSILON) {
		return nlerp(start, end, t);
	}

	gquat delta = start.inverse() * end;
	return ((delta ^ t) * start).normalized();
}





inline gquat mix(const gquat& from, const gquat& to, float t) {
	return from * (1.0f - t) + to * t;
}

inline gquat look_rotation(const gvec3& direction, const gvec3& up) {
	// Find orthonormal basis vectors
	gvec3 f = direction.normalized();
	gvec3 u = up.normalized();
	gvec3 r = cross(u, f);
	u = cross(f, r);
	gquat worldToObject = from_to(gvec3(0, 0, 1), f);
	gvec3 objectUp = worldToObject * gvec3(0, 1, 0);
	gquat u2u = from_to(objectUp, u);
	gquat result = worldToObject * u2u;
	return result.normalized();
}


inline gquat mat4_to_quat(const gmat4& m) {
	gvec3 up = gvec3(m.up.x, m.up.y, m.up.z);
	up.normalize();
	gvec3 forward = gvec3(m.forward.x, m.forward.y, m.forward.z);
	forward.normalize();
	gvec3 right = cross(up, forward);
	up = cross(forward, right);
	return look_rotation(forward, up);
}

inline gquat angle_axis(float angle, const gvec3& axis) noexcept {
	gvec3 norm = axis.normalized();
	float s = sinf(angle * 0.5f);
	return gquat(norm.x * s,
		norm.y * s,
		norm.z * s,
		cosf(angle * 0.5f)
	);
}

inline bool same_orientation(const gquat& l, const gquat& r) {
	return (fabsf(l.x - r.x) <= GQUAT_EPSILON &&
		fabsf(l.y - r.y) <= GQUAT_EPSILON &&
		fabsf(l.z - r.z) <= GQUAT_EPSILON &&
		fabsf(l.w - r.w) <= GQUAT_EPSILON) ||
		(fabsf(l.x + r.x) <= GQUAT_EPSILON &&
			fabsf(l.y + r.y) <= GQUAT_EPSILON &&
			fabsf(l.z + r.z) <= GQUAT_EPSILON &&
			fabsf(l.w + r.w) <= GQUAT_EPSILON);
}



#endif // GQUAT_H