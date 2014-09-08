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
*/

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "singleton.h"
#include "ps2matrix4x4.h"
#include "ps2vector4.h"


#define Pipeline CPipeline::GetSingleton()

class CPipeline : public CSingleton<CPipeline>
{
public:
	CPipeline();
	~CPipeline(){};
	
	inline const Matrix4x4 & GetProjection(void) const;
	inline const Matrix4x4 & GetCamera(void) const;
	inline const Matrix4x4 & GetViewProjection(void) const;
	inline const Matrix4x4 & GetLightDirs(void) const;
	inline const Vector4 & GetLight1Vec(void) const;
	inline const Vector4 & GetLight2Vec(void) const;
	inline const Vector4 & GetLight3Vec(void) const;
	inline const Matrix4x4 & GetLightCols(void) const;
	inline const Vector4 & GetScaleVector(void) const;
	
	inline const Vector4 & GetCameraPos(void) const;
	inline const float GetCameraX(void) const;
	inline const float GetCameraY(void) const;
	inline const float GetCameraZ(void) const;
	inline const float GetCameraXRot(void) const;
	inline const float GetCameraYRot(void) const;
	
	void SetLight1(const Vector4 & Direction, const Vector4 & Colour);
	void SetLight2(const Vector4 & Direction, const Vector4 & Colour);
	void SetLight3(const Vector4 & Direction, const Vector4 & Colour);
	void SetAmbient(const Vector4 & Colour);
	
	void Update(float Strafe, float Advance, float UpDown, float XRot, float YRot);
	void PositionCamera(const Vector4 & Position, const float XRot, const float YRot);
	void PositionCamera(const Vector4 & Position, const Vector4 & LookAt);
	void Initialise(void);

protected:	

	float m_near;		// Near plane
	float m_far;		// Far plane
	float m_FOV;		// Field of View
	float m_Aspect;		// Aspect Ratio
	float m_ScreenW;	// Screen Width
	float m_ScreenH;	// Screen height;
	
	Matrix4x4 m_Projection;
	Matrix4x4 m_Camera;
	Matrix4x4 m_ViewProjection;
	Matrix4x4 m_LightDirs;
	Matrix4x4 m_LightCols;
	
	Vector4 m_Light1Vec;
	Vector4 m_Light2Vec;
	Vector4 m_Light3Vec;
	
	Vector4 m_Scale;
		
	Vector4 m_CamPos;
	float m_XRot, m_YRot;
	
	void CalcProjection(void);	
};

inline const Matrix4x4 & CPipeline::GetProjection(void) const
	{return (m_Projection);}

inline const Matrix4x4 & CPipeline::GetCamera(void) const
	{return (m_Camera);}

inline const Matrix4x4 & CPipeline::GetViewProjection(void) const
	{return (m_ViewProjection);}
	
inline const Matrix4x4 & CPipeline::GetLightDirs(void) const
	{return (m_LightDirs);}
	
inline const Vector4 & CPipeline::GetLight1Vec(void) const
	{return (m_Light1Vec);}
	
inline const Vector4 & CPipeline::GetLight2Vec(void) const
	{return (m_Light2Vec);}
	
inline const Vector4 & CPipeline::GetLight3Vec(void) const
	{return (m_Light3Vec);}
	
inline const Matrix4x4 & CPipeline::GetLightCols(void) const
	{return (m_LightCols);}
	
inline const Vector4 & CPipeline::GetScaleVector(void) const
	{return (m_Scale);}
	
inline const Vector4 & CPipeline::GetCameraPos(void) const
	{return m_CamPos;}
	
inline const float CPipeline::GetCameraX(void) const
	{return m_CamPos.x;}
	
inline const float CPipeline::GetCameraY(void) const
	{return m_CamPos.y;}
	
inline const float CPipeline::GetCameraZ(void) const
	{return m_CamPos.z;}
	
inline const float CPipeline::GetCameraXRot(void) const
		{return m_XRot;}
		
inline const float CPipeline::GetCameraYRot(void) const
	{return m_YRot;}


#endif