/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:11 $
*
*/#include "ps2quaternion.h"
#include "ps2maths.h"

using namespace HSFMaths;

Quaternion::Quaternion(void)
{
}

Quaternion::Quaternion(const float _x, const float _y, const float _z, const float _w)
:w(_w),x(_x),y(_y),z(_z)
{
}

Quaternion::Quaternion(Vector4 V, float theta)
{
	Vector4 Vnorm = V.Normalise();

	x = Vnorm.x * Sin(theta/2.0f);
	y = Vnorm.y * Sin(theta/2.0f);
	z = Vnorm.z * Sin(theta/2.0f);
	w = Cos(theta/2.0f);
}

void Quaternion::SetQuaternion(Vector4 V, const float theta)
{
	Vector4 Vnorm = V.Normalise();

	x = Vnorm.x * Sin(theta/2.0f);
	y = Vnorm.y * Sin(theta/2.0f);
	z = Vnorm.z * Sin(theta/2.0f);
	w = Cos(theta/2.0f);
}

Quaternion::~Quaternion(void)
{
}

Quaternion & Quaternion::NormaliseSelf(void)
{
	float s = 1.0f / Sqrt(w * w + x * x + y * y + z * z);
	w *= s;
	x *= s;
	y *= s;
	z *= s;

	return *this;
}


Quaternion Quaternion::Conjugate(void)
{
	Quaternion quat;
	quat.x = -x;
	quat.y = -y;
	quat.z = -z;
	quat.w = w;
	return quat;
}



Matrix4x4 Quaternion::ToRotationMatrix4x4(void)
{
	return Matrix4x4(	1 - 2 * y * y - 2 * z * z,
		    		 	2 * x * y + 2 * w * z,
				   		2 * x * z - 2 * w * y,
						0,
						2 * x * y - 2 * w * z,
						1 - 2 * x * x - 2 * z * z,
						2 * y * z + 2 * w * x,
						0,
						2 * x * z + 2 * w * y,
						2 * y * z - 2 * w * x,
						1 - 2 * x * x - 2 * y * y,
						0,
						0,0,0,1);
}

void Quaternion::DumpQuaternion(const char * s = NULL)
{
	if(s != NULL)printf("\n%f %f %f %f %s\n\n", x, y, z, w, s);
	else printf("\n%f %f %f %f\n\n", x, y, z, w);
}