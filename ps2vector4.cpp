/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.4 $
* $Date: 2007/08/27 19:37:12 $
*
*/#include "ps2vector4.h"

Vector4::Vector4(void)
{
}

Vector4::Vector4(const float _x, const float _y, const float _z, const float _w)
:x(_x),y(_y),z(_z),w(_w)
{
}

// Copy constructor
Vector4::Vector4(const Vector4 &rhs)
:x(rhs.x),y(rhs.y),z(rhs.z),w(rhs.w)
{
}

Vector4::~Vector4(void)
{
}

Vector4 & Vector4::operator +=(const Vector4 &rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w = 1.0f;

	return *this;
}

Vector4 & Vector4::operator -=(const Vector4 &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w = 1.0f;

	return *this;
}

float Vector4::Dot4(const Vector4 &rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
}

float Vector4::Dot3(const Vector4 &rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z);
}

Vector4 Vector4::Cross(const Vector4 &rhs) const
{
	return Vector4(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x, 1.0f); 
}

Vector4 & Vector4::operator *=(const float s)
{
	x *= s;
	y *= s;
	z *= s;
	w  = 1.0f;

	return *this;
}

Vector4 & Vector4::operator /=(const float s)
{
	x /= s;
	y /= s;
	z /= s;
	w = 1.0f;;

	return *this;
}

float Vector4::Length() const
{
	return Sqrt(x * x + y * y + z * z);
}

float Vector4::LengthSqr() const
{
	return (x * x + y * y + z * z);
}


bool Vector4::operator ==(const Vector4 & rhs) const
{
	return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w));
}

// Return the ZeroVector if the lenth of the vector is very small
Vector4 Vector4::Normalise()
{
	float tmp_Length;
	Vector4 ZeroVector = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	
	tmp_Length = this->Length();
	if(tmp_Length < 0.0001f)
		return ZeroVector;
	
	return (*this / this->Length());
}

// Set to the ZeroVector if the lenth of the vector is very small
void Vector4::NormaliseSelf()
{
	float tmp_Length;
	Vector4 ZeroVector = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	
	tmp_Length = this->Length();
	if(tmp_Length < 0.0001f)
		*this = ZeroVector;
	else
		*this /= this->Length();
}

// return 1 if the vector is a zero vector, 0 otherwise
int Vector4::IsZeroVector(void)
{
	if(x != 0.0)return 0;
	if(y != 0.0)return 0;
	if(z != 0.0)return 0;
	if(w != 0.0)return 0;
	return 1;
}

void Vector4::DumpVector4(char * s)
{
	if(s != NULL)printf("\n%f %f %f %f %s\n\n", x, y, z, w, s);
	else printf("\n%f %f %f %f\n\n", x, y, z, w);
}