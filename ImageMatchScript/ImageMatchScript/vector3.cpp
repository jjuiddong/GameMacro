
#include "global.h"
#include "vector3.h"

using namespace match;


Vector3 Vector3::Min(-FLT_MAX, -FLT_MAX, -FLT_MAX);
Vector3 Vector3::Max(FLT_MAX, FLT_MAX, FLT_MAX);
Vector3 Vector3::Up(0, 1, 0);
Vector3 Vector3::Right(1,0,0);
Vector3 Vector3::Forward(0,0,1);


bool Vector3::IsEmpty() const
{
	return (x==0) && (y==0) && (z==0);
}

float Vector3::Length() const
{
	return sqrt(x*x + y*y + z*z);
}

float Vector3::LengthRoughly(const Vector3 &rhs) const
{
	Vector3 v = *this - rhs;
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

Vector3 Vector3::Normal() const
{
	const float len = Length();
	if (len < 0.001f)
		return Vector3(0,0,0);
	return *this / len;
}


void Vector3::Normalize()
{
	*this = Normal();
}


Vector3 Vector3::operator + () const
{
	return *this;
}


Vector3 Vector3::operator - () const
{
	return Vector3(-x, -y, -z);
}


Vector3 Vector3::operator + ( const Vector3& rhs ) const
{
	return Vector3(x+rhs.x, y+rhs.y, z+rhs.z);
}


Vector3 Vector3::operator - ( const Vector3& rhs ) const
{
	return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
}


Vector3& Vector3::operator += ( const Vector3& rhs )
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}


Vector3& Vector3::operator -= ( const Vector3& rhs )
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}


Vector3& Vector3::operator *= ( const Vector3& rhs )
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	return *this;
}


Vector3& Vector3::operator /= ( const Vector3& rhs )
{
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
	return *this;
}


float Vector3::DotProduct( const Vector3& v ) const
{
	return x * v.x + y * v.y + z * v.z;
}


Vector3 Vector3::CrossProduct( const Vector3& v ) const
{
	return Vector3( 
		(y * v.z) - (z * v.y), 
		(z * v.x) - (x * v.z), 
		(x * v.y) - (y * v.x) );
}


Vector3 Vector3::Interpolate( const Vector3 &v, const float alpha) const
{
	return Vector3(x + (alpha * ( v.x - x ) ),
		y + (alpha * ( v.y - y ) ),
		z + (alpha * ( v.z - z ) ) );
}
