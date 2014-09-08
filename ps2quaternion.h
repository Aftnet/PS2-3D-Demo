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
*/

#ifndef _PS2QUATERNION_H_
#define _PS2QUATERNION_H_

#include <stdio.h>
#include "ps2matrix4x4.h"
#include "ps2vector4.h"

namespace HSFMaths
{
	class Quaternion
	{
	public:
	
		Quaternion(void);
		Quaternion(const float _x, const float _y, const float _z, const float _w);
		Quaternion(Vector4 V, const float theta);
		~Quaternion(void);

		Quaternion & NormaliseSelf(void);
		Quaternion Conjugate(void);
		Matrix4x4 ToRotationMatrix4x4(void);
		void SetQuaternion(Vector4 V, const float theta);
		void Quaternion::DumpQuaternion(const char * s = NULL);

		float w,x,y,z;
	};
}

inline HSFMaths::Quaternion operator + (const HSFMaths::Quaternion &q1,
										const HSFMaths::Quaternion &q2)
{
	return HSFMaths::Quaternion(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

inline HSFMaths::Quaternion operator - (const HSFMaths::Quaternion &q1,
										const HSFMaths::Quaternion &q2)
{
	return HSFMaths::Quaternion(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
}


inline HSFMaths::Quaternion operator * (const HSFMaths::Quaternion &A,
										const HSFMaths::Quaternion &B)
{

  HSFMaths::Quaternion C;

  C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
  C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
  C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
  C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
  return C;
}



inline HSFMaths::Quaternion operator * (const HSFMaths::Quaternion &q,
										const float &s)
{
	return HSFMaths::Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}

inline HSFMaths::Quaternion operator * (const float &s,
										const HSFMaths::Quaternion &q)
{
	return HSFMaths::Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}


#endif
