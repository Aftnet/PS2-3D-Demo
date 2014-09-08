/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.5 $
* $Date: 2007/08/27 20:22:34 $
*
*/

#include <sps2lib.h>
#include <sps2tags.h>
#include <sps2util.h>
#include <sps2regstructs.h>
#include "PS2Defines.h"
#include "ps2matrix4x4.h"
#include "ps2maths.h"
#include "sps2wrap.h"
#include "dma.h"
#include "pipeline.h"


// Singleton initialiser (note: Never make new copies of this classes anywhere else)
CPipeline CPipeline_SingletonInit;

#define ROTXMAX (PIHALF - 0.02f)

CPipeline::CPipeline()
{
	// Don't put anything in here which relies on other classes since
	// this class will probably be initialised first by the singleton 
	// instance.	
}

void CPipeline::Initialise(void)
{
	m_ViewProjection = Matrix4x4::IDENTITY;
	m_LightDirs = Matrix4x4::IDENTITY;
	m_LightCols = Matrix4x4::NULLMATRIX;
	m_Camera = Matrix4x4::IDENTITY;
	m_Projection = Matrix4x4::IDENTITY;
	m_Scale = Vector4(2048.0f, 2048.0f, ((float)0xFFFFFF) / 32.0f, 0.0f);
	
	m_near = 2.0f;				// Near plane
	m_far = 2000.0f;			// Far plane
	m_FOV = 60.0f;				// FOV
	m_Aspect = 4.0f / 3.0f;		// Aspect ratio
	
	m_ScreenW = sps2UScreenGetWidth();
	m_ScreenH = sps2UScreenGetHeight();
	
	assert((m_ScreenW > 0.0f) && (m_ScreenW > 0.0f));
	
	//printf("%f %f\n", m_ScreenW, m_ScreenH);
	
	m_CamPos = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_XRot = 0.0f;
	m_YRot = 0.0f;
	
	CalcProjection();
	
	m_ViewProjection = m_Camera * m_Projection;
	
	// Set ambient light on R=G=B=128
	m_LightCols(3,0) = 128.0f;
	m_LightCols(3,1) = 128.0f;
	m_LightCols(3,2) = 128.0f;
	
}

void CPipeline::CalcProjection(void)
{
	float FovYdiv2 = DegToRad(m_FOV) * 0.5f; 
	float cotFOV = 1.0f / (Sin(FovYdiv2) / Cos(FovYdiv2));
	
	// We will be projecting to the 4096 wide drawing area
	// Look also at the VU projection code
	float w = cotFOV * (m_ScreenW / 4096.0f) / m_Aspect;
	float h = cotFOV * (m_ScreenH / 4096.0f);

	m_Projection = Matrix4x4  (w,  0, 0,                                   0,
						 	   0, -h, 0,                                   0,
						 	   0,  0, (m_far+m_near) / (m_far-m_near),    -1,
						 	   0,  0, (2*m_far*m_near) / (m_far-m_near),   0);
						 
}


void CPipeline::Update(float Strafe, float Advance, float UpDown, float XRot, float YRot)
{
	// Process rotations
	m_YRot -= YRot * Abs(YRot) * 0.07f;
	m_XRot += XRot * Abs(XRot) * 0.07f;
	
	// Clamp xrot to prevent going upside down.
	if(m_XRot > ROTXMAX)
		m_XRot = ROTXMAX;
	
	if(m_XRot < -ROTXMAX)
		m_XRot = -ROTXMAX;
		
	// Keep y rotation in one circle
	if (m_YRot >= TWOPI)	m_YRot -= TWOPI;
	if (m_YRot <= -TWOPI)	m_YRot += TWOPI;
	
	// Sort out the rotations
	Matrix4x4 roty, rotx;
	roty.RotationY( m_YRot );
	rotx.RotationX( m_XRot );
	Matrix4x4 CamRot = rotx * roty;
	
	// Transpose rotation matrix to get camera matrix
	CamRot = Transpose(CamRot);

	// Get the direction vectors for the camera
	Vector4 vXAxis = Vector4(CamRot(0,0), CamRot(1,0), CamRot(2,0), 1);
	Vector4 vYAxis = Vector4(CamRot(0,1), CamRot(1,1), CamRot(2,1), 1);
	Vector4 vZAxis = Vector4(CamRot(0,2), CamRot(1,2), CamRot(2,2), 1);
	
	// Process movement
	Strafe *=   0.5f;
	Advance *=  0.5f;
	UpDown *=   0.5f;

	// Work out the requested move for this frame
	Vector4 vReqMove;
	vReqMove =  vXAxis * Strafe;
	vReqMove += vYAxis * UpDown;
	vReqMove += vZAxis * Advance;
	
	// And add it to the camera position
	m_CamPos += vReqMove;

	// Finally build the camera matrix
	Matrix4x4 matView;
	matView.Translation(-m_CamPos.x, -m_CamPos.y, -m_CamPos.z);
	
	m_Camera = matView * CamRot;
	m_ViewProjection = m_Camera * m_Projection;

}

void CPipeline::PositionCamera(const Vector4 & Position, const float XRot, const float YRot)
{
	m_CamPos = Position;
	m_YRot = YRot;
	m_XRot = XRot;
}

void CPipeline::PositionCamera(const Vector4 & Position, const Vector4 & LookAt)
{
	// Calculate and set the camera matrix
	const Vector4 Up = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	m_Camera.LookAt(Position, LookAt, Up);
	
	// Set the position of the camera
	m_CamPos = Position;
	
	// Now we need to calculate the X and Y rotation variables.
	// First calculate the vector length along the xz plane
	float Hypot = Sqrt((Position.x - LookAt.x)*(Position.x - LookAt.x) 
								   +(Position.z - LookAt.z)*(Position.z - LookAt.z));
	
	// If this vector is too small we are either looking directly up or down
	// Otherwise calculate the X rotation angle from the triangle.
	if(Hypot < 0.01f)
		{
		if((Position.y - LookAt.y) > 0.0f) 
			m_XRot = -ROTXMAX;
		else 
			m_XRot = ROTXMAX;
		}
	else
		{
		float Hypot2 = Sqrt((LookAt.y - Position.y)*(LookAt.y - Position.y) + Hypot * Hypot);
		m_XRot = -ACos(Hypot/Hypot2);
		}	

	// Now lets calculate the rotation round the Y axis
	// Get the vector along the xz plane
	Vector4 Lookxz, ZAxis = Vector4(0.0f, 0.0f, -1.0f, 1.0f);
	Lookxz.x = 	LookAt.x - Position.x;
	Lookxz.y = 	0.0f;
	Lookxz.z = 	LookAt.z - Position.z;
	Lookxz.w = 1.0f;
	//ZAxis.x = 0.0f;
	//ZAxis.y = 0.0f;
	//ZAxis.z = -1.0f;
	//ZAxis.w = 1.0f;
	
	// Normalise the xz plane vector
	Lookxz.NormaliseSelf();
	
	// If this vector is too small we are looking directly up or down
	// so just set m_YRot to 0.0 and get out of here!
	// if(Lookxz.x == 0.0f && Lookxz.y == 0.0f && Lookxz.z == 0.0f && Lookxz.w == 0.0f)
	if(Lookxz.IsZeroVector())
		{
		m_YRot = 0.0f;
		return;
		}
	
	// Take the Dot product between the -ve z asix and the xz vector, this == cos(m_YRot)	
	float Dot = ZAxis.Dot3(Lookxz);
	
	// Get Y rotation and make sure the sign is correct.
	m_YRot = ACos(Dot);	
	if((LookAt.x - Position.x) > 0.0f)m_YRot = -m_YRot;
}

void CPipeline::SetLight1(const Vector4 & Direction, const Vector4 & Colour)
{
	// Set the direction
	Vector4 Temp = Direction;
	assert(Temp.LengthSqr() > 0.0f);
	Temp.NormaliseSelf();
	m_LightDirs(0,0) = Temp.x;
	m_LightDirs(1,0) = Temp.y;
	m_LightDirs(2,0) = Temp.z;

	m_Light1Vec = Temp;
	
	// Set the colour
	m_LightCols(0,0) = Colour.x;
	m_LightCols(0,1) = Colour.y;
	m_LightCols(0,2) = Colour.z;
}

void CPipeline::SetLight2(const Vector4 & Direction, const Vector4 & Colour)
{
	// Set the direction
	Vector4 Temp = Direction;
	assert(Temp.LengthSqr() > 0.0f);
	Temp.NormaliseSelf();
	m_LightDirs(0,1) = Temp.x;
	m_LightDirs(1,1) = Temp.y;
	m_LightDirs(2,1) = Temp.z;

	m_Light2Vec = Temp;
	
	// Set the colour
	m_LightCols(1,0) = Colour.x;
	m_LightCols(1,1) = Colour.y;
	m_LightCols(1,2) = Colour.z;
}

void CPipeline::SetLight3(const Vector4 & Direction, const Vector4 & Colour)
{
	// Set the direction
	Vector4 Temp = Direction;
	assert(Temp.LengthSqr() > 0.0f);
	Temp.NormaliseSelf();
	m_LightDirs(0,2) = Temp.x;
	m_LightDirs(1,2) = Temp.y;
	m_LightDirs(2,2) = Temp.z;

	m_Light3Vec = Temp;
	
	// Set the colour
	m_LightCols(2,0) = Colour.x;
	m_LightCols(2,1) = Colour.y;
	m_LightCols(2,2) = Colour.z;
}

void CPipeline::SetAmbient(const Vector4 & Colour)
{
	// Set the colour
	m_LightCols(3,0) = Colour.x;
	m_LightCols(3,1) = Colour.y;
	m_LightCols(3,2) = Colour.z;

	//m_LightDirs.DumpMatrix4x4();
	//m_LightCols.DumpMatrix4x4();
}