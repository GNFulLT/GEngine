#ifndef GVEC3_H
#define GVEC3_H

#define GVEC3_EPSILON 0.000001f

#include <cmath>

struct gvec3
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float v[3];
	};

	inline gvec3() : x(0),y(0),z(0) {}
	inline gvec3(float _x,float _y, float _z) : x(_x),y(_y),z(_z){}
	inline gvec3(const float* begin) : x(begin[0]),y(begin[1]),z(begin[2]) {}

	inline float len_sq() const noexcept
	{
		return x * x + y * y + z * z;
	}

	inline float len() const noexcept
	{
		float lenSq = x * x + y * y + z * z;
		if (lenSq < GVEC3_EPSILON) {
			return 0.0f;
		}
		return sqrtf(lenSq);
	}

	inline void normalize()
	{
		float lenSq = x * x + y * y + z * z;
		if (lenSq < GVEC3_EPSILON) { return; }
		float invLen = 1.0f / sqrtf(lenSq);
		x *= invLen;
		y *= invLen;
		z *= invLen;
	}

	inline gvec3 normalized() const noexcept
	{
		float lenSq = x * x + y * y + z * z;
		if (lenSq < GVEC3_EPSILON) { return gvec3(); }
		float invLen = 1.0f / sqrtf(lenSq);
		return gvec3(
			x * invLen,
			y * invLen,
			z * invLen
		);
	}
};

inline void operator*=(gvec3 & l, float r) noexcept
{
	l.x *= r;
	l.y *= r;
	l.z *= r;
}

inline void operator-=(gvec3& l, const gvec3& r) noexcept
{
	l.x -= r.x;
	l.y -= r.y;
	l.z -= r.z;
}

inline void operator+=(gvec3& l, const gvec3& r) noexcept
{
	l.x += r.x;
	l.y += r.y;
	l.z += r.z;
}

inline gvec3 operator+(const gvec3& l, const gvec3& r) noexcept
{
	return gvec3(l.x + r.x, l.y + r.y, l.z + r.z);
}

inline gvec3 operator-(const gvec3& l, const gvec3& r) noexcept
{
	return gvec3(l.x - r.x, l.y - r.y, l.z - r.z);
}

inline gvec3 operator*(const gvec3& l, const float r) noexcept
{
	return gvec3(l.x * r, l.y * r, l.z * r);
}

inline gvec3 operator*(const gvec3& l, const gvec3& r) noexcept
{
	return gvec3(l.x * r.x, l.y * r.y, l.z * r.z);
}

inline float dot(const gvec3& l, const gvec3& r) noexcept
{
	return (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
}

inline float angle(const gvec3& l, const gvec3& r)
{
	float leftLen = l.x * l.x + l.y * l.y + l.z * l.z;
	float rightLen = r.x * r.x + r.y * r.y + r.z * r.z;

	if (leftLen < GVEC3_EPSILON || rightLen < GVEC3_EPSILON) { return 0; }

	int up = (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
	int down = sqrtf(leftLen) * sqrtf(rightLen);
	return acosf(up / down);
}

inline float angle_cosf(const gvec3& l, const gvec3& r)
{
	float leftLen = l.x * l.x + l.y * l.y + l.z * l.z;
	float rightLen = r.x * r.x + r.y * r.y + r.z * r.z;

	if (leftLen < GVEC3_EPSILON || rightLen < GVEC3_EPSILON) { return 0; }

	int up = (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
	int down = sqrtf(leftLen) * sqrtf(rightLen);
	return up / down;
}

inline gvec3 project(const gvec3& vec, const gvec3& proj)
{
	float projLen = proj.len();
	if (projLen == 0)
		return gvec3();
	
	return vec * (dot(vec, proj) / projLen);
}

inline gvec3 reject(const gvec3& vec, const gvec3& proj)
{
	gvec3 proj2 = project(vec, proj);
	return vec - proj2;
}

inline gvec3 bounce_reflect(const gvec3& vec, const gvec3& normal)
{
	float normLen = normal.len();
	if (normLen == 0)
		return gvec3();

	return vec - (normal * (dot(vec,normal)/normLen) * 2.f);
}

inline gvec3 cross(const gvec3& l, const gvec3& r)
{
	return gvec3(
		l.y * r.z - l.z * r.y,
		l.z * r.x - l.x * r.z,
		l.x * r.y - l.y * r.x
	);
}

inline gvec3 lerp(const gvec3& s, const gvec3& e, float t) {
	return gvec3(
		s.x + (e.x - s.x) * t,
		s.y + (e.y - s.y) * t,
		s.z + (e.z - s.z) * t
	);
}

inline gvec3 slerp(const gvec3& s, const gvec3& e, float t) {
	if (t < 0.01f) {
		return lerp(s, e, t);
	}
	gvec3 from = s.normalized();
	gvec3 to = e.normalized();
	float theta = angle(from, to);
	float sin_theta = sinf(theta);
	float a = sinf((1.0f - t) * theta) / sin_theta;
	float b = sinf(t * theta) / sin_theta;
	return from * a + to * b;
}

inline gvec3 nlerp(const gvec3& s, const gvec3& e, float t) {
	gvec3 linear(
		s.x + (e.x - s.x) * t,
		s.y + (e.y - s.y) * t,
		s.z + (e.z - s.z) * t
	);
	return linear.normalized();
}

inline bool operator==(const gvec3& l, const gvec3& r) {
	gvec3 diff(l - r);
	return diff.len_sq() < GVEC3_EPSILON;
}
inline bool operator!=(const gvec3& l, const gvec3& r) {
	return !(l == r);
}
#endif // GVEC3_H