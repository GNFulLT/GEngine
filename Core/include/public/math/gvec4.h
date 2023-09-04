#ifndef GVEC4_H
#define GVEC4_H

#define GVEC4_EPSILON 0.000001f
#define GPI 3.14159265359f
struct gvec4
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
		float v[4];
	};
	inline gvec4() : x(0),y(0),z(0),w(0) {}
	inline gvec4(float _x,float _y,float _z,float _w) : x(_x),y(_y),z(_z),w(_w) {}
	inline gvec4(float* vec4) : x(vec4[0]),y(vec4[1]),z(vec4[2]),w(vec4[3]) {}

};

#endif // GVEC4_H