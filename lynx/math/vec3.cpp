#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "mathconst.h"
#include "vec3.h"
#include "matrix.h"
#ifdef _DEBUG
#include <stdio.h> // fprintf
#endif

const vec3_t vec3_t::origin(0,0,0);

vec3_t::vec3_t()
{
	x=y=z=0;
}

vec3_t::vec3_t(float nx, float ny, float nz)
{
	x=nx;
	y=ny;
	z=nz;
}

vec3_t::vec3_t(float all)
{
	x = y = z = all;
}

float vec3_t::Abs(void) const
{
	return (float)sqrt(x*x+y*y+z*z);
}

float vec3_t::AbsSquared(void) const
{
	return x*x+y*y+z*z;
}

float vec3_t::Normalize(void)
{
	float length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrtf(length);

	if(length > lynxmath::EPSILON)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}

void vec3_t::SetLength(float scalelen)
{
	float length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrtf(length);

	if(length > lynxmath::EPSILON)
	{
		ilength = scalelen/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
}

vec3_t vec3_t::Normalized(void) const
{
	float f = Abs();
	if(f < lynxmath::EPSILON)
		return vec3_t(0, 0, 0);
	return (*this)*(1.0f/f);
}

bool vec3_t::IsNormalized() const
{
	return fabsf(Abs()-1.0f) < lynxmath::EPSILON;
}

bool vec3_t::IsNull() const
{
	return x==0.0f && y==0.0f && z==0.0f;
}

bool vec3_t::IsInArea(const vec3_t& min, const vec3_t& max) const
{
	return	x >= min.x && x <= max.x && 
			y >= min.y && y <= max.y &&
			z >= min.z && z <= max.z;
}

vec3_t &vec3_t::operator +=(const vec3_t &v)
{
	x+= v.x;
	y+= v.y;
	z+= v.z;
	return *this;
}

vec3_t &vec3_t::operator -=(const vec3_t &v)
{
	x-= v.x;
	y-= v.y;
	z-= v.z;
	return *this;
}

vec3_t &vec3_t::operator *=(const float &f)
{
	x*=f;
	y*=f;
	z*=f;
	return *this;
}

vec3_t &vec3_t::operator /=(const float &f)
{
	float invf = 1/f;
	x*=f;
	y*=f;
	z*=f;
	return *this;
}

vec3_t vec3_t::rand(float mx, float my, float mz)
{
	x = (float)(::rand()%1000);
	x*=.001f;
	y = (float)(::rand()%1000);
	y*=.001f;
	z = (float)(::rand()%1000);
	z*=.001f;
	x*=mx;
	y*=my;
	z*=mz;
	return *this;
}

vec3_t vec3_t::operator -(void) const
{
	return vec3_t(-x, -y, -z);
}

const vec3_t operator+(vec3_t const &a, vec3_t const &b)
{
	return vec3_t(
		a.x+b.x,
		a.y+b.y,
		a.z+b.z
	);
}

const vec3_t operator-(vec3_t const &a, vec3_t const &b)
{
	return vec3_t(
		a.x-b.x,
		a.y-b.y,
		a.z-b.z
	);
}

const vec3_t operator*(vec3_t const &v, float const &f)
{
	return vec3_t(
		v.x*f,
		v.y*f,
		v.z*f
	);
}

const vec3_t operator*(float const &f, vec3_t const &v)
{
	return vec3_t(
		v.x*f,
		v.y*f,
		v.z*f
	);
}

const vec3_t operator/(vec3_t const &v, float const &f)
{
	float fi = 1.0f / f;
	return v*fi;
}

// cross product
const vec3_t operator^(vec3_t const &a, vec3_t const &b)
{
	return vec3_t(
		(a.y*b.z-a.z*b.y),
		(a.z*b.x-a.x*b.z),
		(a.x*b.y-a.y*b.x)
	);
}

// dot product
const float operator*(vec3_t const &a, vec3_t const &b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

bool operator==(vec3_t const &a, vec3_t const &b)
{
	if(fabs(a.x-b.x) < lynxmath::EPSILON)
		if(fabs(a.y-b.y) < lynxmath::EPSILON)
			if(fabs(a.z-b.z) < lynxmath::EPSILON)
				return true;
	return false;
}

bool operator!=(vec3_t const &a, vec3_t const &b)
{
	return  (fabs(a.x-b.x) > lynxmath::EPSILON) ||
			(fabs(a.y-b.y) > lynxmath::EPSILON) ||
			(fabs(a.z-b.z) > lynxmath::EPSILON);
}

float vabs(vec3_t v)
{
	return v.Abs();
}

void vec3_t::AngleVec3(const vec3_t& angles, vec3_t* forward, vec3_t* up, vec3_t* side)
{
	float sx, cx;
	float sy, cy;
	float sz, cz;
	float x, y, z;

	x = angles.x * lynxmath::DEGTORAD;
	y = angles.y * lynxmath::DEGTORAD;
	z = angles.z * lynxmath::DEGTORAD;

	sx = sinf(x);
	cx = cosf(x);
	sy = sinf(y);
	cy = cosf(y);
	sz = sinf(z);
	cz = cosf(z);

	if(forward)
	{
		forward->x = -cx*sy;
		forward->y = sx;
		forward->z = -cx*cy;
	}

	if(up)
	{
		up->x = cz*sx*sy-sz*cy;
		up->y = cx*cz;
		up->z = sy*sz+sx*cy*cz;
	}

	if(side)
	{
		side->x = sx*sy*sz+cy*cz;
		side->y = cx*sz;
		side->z = sx*sz*cy-cz*sy;
	}
}

float vec3_t::GetAngleDeg(vec3_t& a, vec3_t& b)
{
	float d = a.Abs() * b.Abs();
#ifdef _DEBUG
	if(fabsf(d) < lynxmath::EPSILON)
	{
		assert(0);
		return 0.0f;
	}
#endif
	return acosf((a * b) / d) * lynxmath::RADTODEG;
}

#ifdef _DEBUG
void vec3_t::Print()
{
	fprintf(stderr, "%.2f %.2f %.2f\n", x, y, z);
}
#endif

vec3_t vec3_t::Lerp(const vec3_t& p1, const vec3_t& p2, const float f)
{
    return (1-f)*p1 + f*p2;
}

bool vec3_t::RayCylinderIntersect(const vec3_t& pStart, const vec3_t& pEnd, 
                                  const vec3_t& edgeStart, const vec3_t& edgeEnd,
                                  const float radius,
                                  float* f)
{
    // math. for 3d game programming 2nd ed. page 270
    const vec3_t pv = pEnd - pStart;
    const vec3_t pa = edgeEnd - edgeStart;
    const vec3_t s0 = pStart - edgeStart;
    const float pa_squared = pa.AbsSquared();
    
    // a
    const float pva = pv * pa;
    const float a = pv.AbsSquared() - pva*pva/pa_squared;

    // b
    const float b = s0*pv - (s0*pa)*(pva)/pa_squared;

    // c
    const float ps0a = s0*pa;
    const float c = s0.AbsSquared() - radius*radius - ps0a*ps0a/pa_squared;

    const float dis = b*b - a*c;
    if(dis < 0)
    {
        *f = 999999.9f;
        return false;
    }

    *f = (-b - sqrt(dis))/a;
    const float collision = (pStart + *f*pv - edgeStart)*pa;

    return collision >= 0 && collision <= pa_squared;
}

bool vec3_t::RaySphereIntersect(const vec3_t& pStart, const vec3_t& pEnd,
                                const vec3_t& pSphere, const float radius,
                                float* f)
{
    const float rsquared = radius*radius;
	const float a = (pEnd.x - pStart.x)*(pEnd.x - pStart.x) + (pEnd.y - pStart.y)*(pEnd.y - pStart.y) + (pEnd.z - pStart.z)*(pEnd.z - pStart.z);
	const float b = 2*( (pEnd.x - pStart.x)*(pStart.x - pSphere.x) + (pEnd.y - pStart.y)*(pStart.y - pSphere.y) + (pEnd.z - pStart.z)*(pStart.z - pSphere.z) );
	const float c = pSphere.x*pSphere.x + pSphere.y*pSphere.y + pSphere.z*pSphere.z + pStart.x*pStart.x + pStart.y*pStart.y + pStart.z*pStart.z - 2*(pSphere.x*pStart.x + pSphere.y*pStart.y + pSphere.z*pStart.z) - rsquared;
	const float discrsquare = b*b - 4*a*c;

	if(discrsquare <= 0)
		return false;

	const float discr = sqrt(discrsquare);
	const float u1 = (-b + discr)/(2*a);
	const float u2 = -(b + discr)/(2*a);
	*f = u1 < u2 ? u1 : u2;
	return *f > 0.0f && *f < 1.0f;
}