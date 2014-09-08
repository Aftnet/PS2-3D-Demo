/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:10 $
*
*/

#ifndef __MS3DMODEL_H__
#define __MS3DMODEL_H__

#include <stdio.h>
#include "pipeline.h"
#include "ps2matrix4x4.h"
#include "ms3dff.h"

typedef struct vertstr
{
	Vector4 position;
	Vector4 normal;
	Vector4 texuv;
} vertex;

typedef struct tristr
{
	vertex vert[3];
	Vector4 realnormal;
} triangle;

typedef struct grpstr
{
	//Material Information
	float ambient[4];                         //
    float diffuse[4];                         //
    float specular[4];                        //
    float emissive[4];                        //
    float shininess;                          // 0.0f - 128.0f
    float transparency; 
	//Triangle Information
	int numtriangles;
	struct tristr *ptris;
} group;

class CMs3dModel
{
public:
	CMs3dModel(void);
	
	~CMs3dModel(void);
	
	void SetWorldMatrix(const Matrix4x4 & matWorld);

	bool LoadModelData(const char * strFilename, bool bKeepTriInfo, bool bTransparent = false);
	
	triangle* CreateTriInfoArray();
	
	void DeleteTriInfo();
	
	const void PrintModelData();
	
	const int GetTotalNumTriangles();
	
	void Render(void);

protected:

	void InitialiseDMA(bool bTransparent);
	
	// The world and WorldViewProjection Matrices
	Matrix4x4 m_World, m_WVP;
	
	//Model data:
	int m_iNumGroups;
	int m_iTotalNumTriangles;
	group *m_pGroups;
		
	bool m_bWorldLoaded;
	bool m_bDataLoaded;
	bool m_bTriInfoPresent;
	
	//Array of pointers to Static DMA chain chunks
	int *m_pStaticAddrArr;
};

#endif
