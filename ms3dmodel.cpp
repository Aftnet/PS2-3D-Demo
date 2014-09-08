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


#include "ms3dmodel.h"
#include "dma.h"
#include <memory.h>
#include <fstream.h>
#include "ms3dff.h"

// Uncomment next line to check loading of model data
//#define MSVERBOSE


CMs3dModel::CMs3dModel(void)
{	
	// Initialise some default vaules
	m_iNumGroups = 0;
	m_pGroups = NULL;
	m_pStaticAddrArr = NULL;
	m_World = Matrix4x4::IDENTITY;
	m_WVP = Matrix4x4::IDENTITY;
	m_bWorldLoaded = false;
	m_bDataLoaded = false;	
}

CMs3dModel::~CMs3dModel(void)
{
	int i;
	if(m_bTriInfoPresent)
	{
		for(i=0;i<m_iNumGroups;i++)
		{
			delete [] m_pGroups[i].ptris;
		}
	}
	delete [] m_pGroups;
	delete [] m_pStaticAddrArr;
}

void CMs3dModel::SetWorldMatrix(const Matrix4x4 & matWorld)
{
	m_World = matWorld;
	m_WVP = m_World * Pipeline.GetViewProjection();
	m_bWorldLoaded = true;
}

bool CMs3dModel::LoadModelData(const char *strFilename, bool bKeepTriInfo, bool bTransparent)
{
	//Stage 1: Read binary file into temporary structures in main memory
	
	ms3d_header header;
	word NumVertices;
	ms3d_vertex *pvert;
	word NumTriangles;
	ms3d_triangle *ptri;
	word NumGroups;
	ms3d_group *pgrp;
	word NumMaterials;
	ms3d_material *pmat;

	int i,j,k;
	m_iNumGroups = 0;
	m_pGroups = NULL;
	fstream file;
	file.open(strFilename,fstream::in | fstream::binary);
	if(!file.is_open())
	{
		cout << "Error opening file " << strFilename << "\n";
		return false;
	}
	else
	{	//Due to strange alignment issues (header structure should have a sizeof 14 but is 16) each member will be read separately
		//Read header
		file.read((char*)(&header.id),10*sizeof(char));
		file.read((char*)(&header.version),sizeof(int));
		//Read number of vertices
		file.read((char*)(&NumVertices),sizeof(word));
		//Allocate space for vertices information
		pvert = new ms3d_vertex[NumVertices];
		//Read vertex information
		for(i=0; i<NumVertices; i++)
		{
			file.read((char*)&pvert[i].flags,sizeof(byte));
			file.read((char*)&pvert[i].vertex,3*sizeof(float));
			file.read((char*)&pvert[i].boneId,sizeof(char));
			file.read((char*)&pvert[i].referenceCount,sizeof(byte));
		}
		//Read number of triangles
		file.read((char*)(&NumTriangles),sizeof(word));
		//Allocate space for triangles information
		ptri = new ms3d_triangle[NumTriangles];
		//Read triangle information
		for(i=0; i<NumTriangles; i++)
		{
			file.read((char*)&ptri[i].flags,sizeof(word));
			file.read((char*)&ptri[i].vertexIndices,3*sizeof(word));
			file.read((char*)&ptri[i].vertexNormals,9*sizeof(float));
			file.read((char*)&ptri[i].s,3*sizeof(float));
			file.read((char*)&ptri[i].t,3*sizeof(float));
			file.read((char*)&ptri[i].smoothingGroup,sizeof(byte));
			file.read((char*)&ptri[i].groupIndex,sizeof(byte));
		}
		//Read number of groups
		file.read((char*)(&NumGroups),sizeof(word));
		//Allocate space for groups information
		pgrp = new ms3d_group[NumGroups];
		//Read group information
		for(i=0; i<NumGroups; i++)
		{
			file.read((char*)&pgrp[i].flags,sizeof(byte));
			file.read((char*)&pgrp[i].name,32*sizeof(char));
			file.read((char*)&pgrp[i].numtriangles,sizeof(word));
			pgrp[i].triangleIndices = new word[pgrp[i].numtriangles];
			for(j=0; j<pgrp[i].numtriangles; j++)
			{
				file.read((char*)&pgrp[i].triangleIndices[j],sizeof(word));
			}
			file.read((char*)&pgrp[i].materialIndex,sizeof(char));
		}
		//Read number of materials
		file.read((char*)(&NumMaterials),sizeof(word));
		//Allocate space for material information
		pmat = new ms3d_material[NumMaterials];
		//Read material information
		for(i=0; i<NumMaterials; i++)
		{
			file.read((char*)&pmat[i].name,32*sizeof(char));
			file.read((char*)&pmat[i].ambient,4*sizeof(float));
			file.read((char*)&pmat[i].diffuse,4*sizeof(float));
			file.read((char*)&pmat[i].specular,4*sizeof(float));
			file.read((char*)&pmat[i].emissive,4*sizeof(float));
			file.read((char*)&pmat[i].shininess,sizeof(float));
			file.read((char*)&pmat[i].transparency,sizeof(float));
			file.read((char*)&pmat[i].mode,sizeof(char));
			file.read((char*)&pmat[i].texture,128*sizeof(char));
			file.read((char*)&pmat[i].alphamap,128*sizeof(char));
		}
		//Close model file
		file.close();
		
		//Stage 2: copy data from temporary structures into class ones discarding unneeded information
		m_iNumGroups = int(NumGroups);
		m_pGroups = new group[m_iNumGroups];
		for(i=0; i<m_iNumGroups; i++)
		{
			//Copy material properties for the group
			if(int(pgrp[i].materialIndex)>=0)
			{
				for(j=0;j<4;j++)
				{
					m_pGroups[i].ambient[j] = float(pmat[int(pgrp[i].materialIndex)].ambient[j]);
					m_pGroups[i].diffuse[j] = float(pmat[int(pgrp[i].materialIndex)].diffuse[j]);
					m_pGroups[i].specular[j] = float(pmat[int(pgrp[i].materialIndex)].specular[j]);
					m_pGroups[i].emissive[j] = float(pmat[int(pgrp[i].materialIndex)].emissive[j]);
				}
				m_pGroups[i].shininess = float(pmat[int(pgrp[i].materialIndex)].shininess);
				m_pGroups[i].transparency = float(pmat[int(pgrp[i].materialIndex)].transparency);
			}
			else
			{
				for(j=0;j<3;j++)
				{
					m_pGroups[i].ambient[j] = 0.5f;
					m_pGroups[i].diffuse[j] = 0.5f;
					m_pGroups[i].specular[j] = 0.5f;
					m_pGroups[i].emissive[j] = 0.5f;
				}
				m_pGroups[i].ambient[3] = 1.0f;
				m_pGroups[i].diffuse[3] = 1.0f;
				m_pGroups[i].specular[3] = 1.0f;
				m_pGroups[i].emissive[3] = 1.0f;
				m_pGroups[i].shininess = 0.0;
				m_pGroups[i].transparency = 1.0;
			}
			//Copy triangle information for the group
			m_pGroups[i].numtriangles = int(pgrp[i].numtriangles);
			m_pGroups[i].ptris = new triangle[m_pGroups[i].numtriangles];
			for(j=0;j<m_pGroups[i].numtriangles;j++)
			{
				for(k=0;k<3;k++)
				{
					//Vertex Positions
					m_pGroups[i].ptris[j].vert[k].position.x=float(pvert[ptri[pgrp[i].triangleIndices[j]].vertexIndices[k]].vertex[0]);
					m_pGroups[i].ptris[j].vert[k].position.y=float(pvert[ptri[pgrp[i].triangleIndices[j]].vertexIndices[k]].vertex[1]);
					m_pGroups[i].ptris[j].vert[k].position.z=float(pvert[ptri[pgrp[i].triangleIndices[j]].vertexIndices[k]].vertex[2]);
					m_pGroups[i].ptris[j].vert[k].position.w=1.0f;
					//Normals
					m_pGroups[i].ptris[j].vert[k].normal.x=float(ptri[pgrp[i].triangleIndices[j]].vertexNormals[k][0]);
					m_pGroups[i].ptris[j].vert[k].normal.y=float(ptri[pgrp[i].triangleIndices[j]].vertexNormals[k][1]);
					m_pGroups[i].ptris[j].vert[k].normal.z=float(ptri[pgrp[i].triangleIndices[j]].vertexNormals[k][2]);
					m_pGroups[i].ptris[j].vert[k].normal.w=0.0f;
					//ST Texture Coordinates
					m_pGroups[i].ptris[j].vert[k].texuv.x=float(ptri[pgrp[i].triangleIndices[j]].s[k]);
					m_pGroups[i].ptris[j].vert[k].texuv.y=float(ptri[pgrp[i].triangleIndices[j]].t[k]);
					m_pGroups[i].ptris[j].vert[k].texuv.z=0.0f;
					m_pGroups[i].ptris[j].vert[k].texuv.w=0.0f;
				}
				//Calculate real geometrical normal for triangle (useful for collision detection)
				m_pGroups[i].ptris[j].realnormal=Normalise(CrossProduct((m_pGroups[i].ptris[j].vert[1].position - m_pGroups[i].ptris[j].vert[0].position),(m_pGroups[i].ptris[j].vert[2].position - m_pGroups[i].ptris[j].vert[1].position)));
			}
		}
		
		//Stage 3: Cleanup unused temporary data
		delete [] pvert;
		delete [] pgrp;
		delete [] ptri;
		delete [] pmat;
		
		//Stage 4: Create the static parts of the model's DMA chain
		InitialiseDMA(bTransparent);
		m_bDataLoaded = true;
		
		//Stage 5: Calculate Model's total triangle number
		j=0;
		for(i=0;i<m_iNumGroups;i++)
		{
			j+=m_pGroups[i].numtriangles;
		}
		m_iTotalNumTriangles=j;
		
		//Stage 6: Delete triangle information if not needed (the rendering code only needs the data already stored in the static DMA chain)
		m_bTriInfoPresent = bKeepTriInfo;
		if(!m_bTriInfoPresent)
		{
			for(i=0;i<m_iNumGroups;i++)
			{
				delete [] m_pGroups[i].ptris;
			}
		}
		return true;
	}
}

triangle * CMs3dModel::CreateTriInfoArray()
{
	int i,j,k;
	triangle *ArrPtr = NULL;
	if(m_bTriInfoPresent)
	{
		j=0;
		k=0;
		for(i=0;i<m_iNumGroups;i++)
		{
			j+=m_pGroups[i].numtriangles;
		}
		ArrPtr = new tristr[j];
		for(i=0;i<m_iNumGroups;i++)
		{
			for(j=0;j<m_pGroups[i].numtriangles;j++)
			{
				ArrPtr[k]=m_pGroups[i].ptris[j];
				k++;
			}
		}
	}
	return ArrPtr;
}

void CMs3dModel::DeleteTriInfo()
{
	if(m_bTriInfoPresent)
	{
		int i;
		for(i=0;i<m_iNumGroups;i++)
		{
			delete [] m_pGroups[i].ptris;
		}
		m_bTriInfoPresent = false;
	}
}

const void CMs3dModel::PrintModelData()
{
	int i,j,k;
	for(i=0; i<m_iNumGroups; i++)
	{
		cout << "Group " << i << ":\n\n";
		cout << "Material Properties:\n";
		cout << "ambient: " << m_pGroups[i].ambient[0] << "," << m_pGroups[i].ambient[1] << "," << m_pGroups[i].ambient[2] << "," << m_pGroups[i].ambient[3] << "\n"; 
		cout << "diffuse: " << m_pGroups[i].diffuse[0] << "," << m_pGroups[i].diffuse[1] << "," << m_pGroups[i].diffuse[2] << "," << m_pGroups[i].diffuse[3] << "\n"; 
		cout << "specular: " << m_pGroups[i].specular[0] << "," << m_pGroups[i].specular[1] << "," << m_pGroups[i].specular[2] << "," << m_pGroups[i].specular[3] << "\n"; 
		cout << "emissive: " << m_pGroups[i].emissive[0] << "," << m_pGroups[i].emissive[1] << "," << m_pGroups[i].emissive[2] << "," << m_pGroups[i].emissive[3] << "\n"; 
		cout << "shininess: " << m_pGroups[i].shininess << "\n";
		cout << "transparency: " << m_pGroups[i].transparency << "\n\n";
		cout << "Triangles: " << m_pGroups[i].numtriangles << "\n";
		for(j=0; j<m_pGroups[i].numtriangles;j++)
		{
			cout << "\nTriangle " << j << "\n";
			for(k=0; k<3; k++)
			{
				cout << "vertex" << k << "\n";
				cout << "position: " << m_pGroups[i].ptris[j].vert[k].position.x << "," << m_pGroups[i].ptris[j].vert[k].position.y << "," << m_pGroups[i].ptris[j].vert[k].position.z << "," << m_pGroups[i].ptris[j].vert[k].position.w << "\n";
				cout << "normal: " << m_pGroups[i].ptris[j].vert[k].normal.x << "," << m_pGroups[i].ptris[j].vert[k].normal.y << "," << m_pGroups[i].ptris[j].vert[k].normal.z << "," << m_pGroups[i].ptris[j].vert[k].normal.w << "\n";
				cout << "texuv: " << m_pGroups[i].ptris[j].vert[k].texuv.x << "," << m_pGroups[i].ptris[j].vert[k].texuv.y << "," << m_pGroups[i].ptris[j].vert[k].texuv.z << "," << m_pGroups[i].ptris[j].vert[k].texuv.w << "\n";
			}
		}
	}
}

const int CMs3dModel::GetTotalNumTriangles()
{
	return m_iTotalNumTriangles;
}

void CMs3dModel::InitialiseDMA(bool bTransparent)
{
	int iCurrentGroup,iNumTrisPerChunk,iNumTris,iAbsoluteTri,iTrisThisChunk,iTriangle,iShinIndex;
	bool bFirst;
	triangle temptri;
	m_pStaticAddrArr = new int [m_iNumGroups];
	for (iCurrentGroup=0; iCurrentGroup<m_iNumGroups; iCurrentGroup++)
	{
		m_pStaticAddrArr[iCurrentGroup] = VIFStaticDMA.GetPointer();
		VIFStaticDMA.Add32(FLUSH);			// Make sure VU1 isn't busy
		VIFStaticDMA.Add32(STCYCL(1,1));	// Unpack linearly, i.e. don't skip any spaces
		VIFStaticDMA.Add32(BASE(42));		// The double buffers start at VU1 address 20 (giving us 20 QW to store data that won't change)
		VIFStaticDMA.Add32(OFFSET(492));	// The size of each buffer.

		VIFStaticDMA.AddUnpack(V4_32, 0, 1);	// Unpack 8QW of data that won't change
		VIFStaticDMA.AddVector(Pipeline.GetScaleVector());	// 0: The scales
		
		//Calculate Shininess index (exponent of power of 2 closest to real shininess index)
		iShinIndex=0;
		while(m_pGroups[iCurrentGroup].shininess>=1.0f)
		{
			m_pGroups[iCurrentGroup].shininess = m_pGroups[iCurrentGroup].shininess/2.0f;
			iShinIndex++;
		}
		
		VIFStaticDMA.AddUnpack(V4_32, 21, 5);
		VIFStaticDMA.AddVector(Vector4(m_pGroups[iCurrentGroup].ambient[0],m_pGroups[iCurrentGroup].ambient[1],m_pGroups[iCurrentGroup].ambient[2],m_pGroups[iCurrentGroup].ambient[3]));
		VIFStaticDMA.AddVector(Vector4(m_pGroups[iCurrentGroup].diffuse[0],m_pGroups[iCurrentGroup].diffuse[1],m_pGroups[iCurrentGroup].diffuse[2],m_pGroups[iCurrentGroup].diffuse[3]));
		VIFStaticDMA.AddVector(Vector4(m_pGroups[iCurrentGroup].specular[0],m_pGroups[iCurrentGroup].specular[1],m_pGroups[iCurrentGroup].specular[2],m_pGroups[iCurrentGroup].specular[3]));
		VIFStaticDMA.AddVector(Vector4(m_pGroups[iCurrentGroup].emissive[0],m_pGroups[iCurrentGroup].emissive[1],m_pGroups[iCurrentGroup].emissive[2],m_pGroups[iCurrentGroup].transparency));
		//Add Shininess Index
		VIFStaticDMA.Add128(iShinIndex);
		
		iNumTrisPerChunk = 27;
		bFirst = true;
		//MSMesh & pMesh = m_pMeshes[nMesh];
		iNumTris = m_pGroups[iCurrentGroup].numtriangles;
		iAbsoluteTri = 0;
		
		while(iNumTris > 0)
		{
	
			if(iNumTris > iNumTrisPerChunk)
			{
				iTrisThisChunk = iNumTrisPerChunk;
				iNumTris -= iTrisThisChunk;
			}
			else
			{
				iTrisThisChunk = iNumTris;
				iNumTris -= iTrisThisChunk;
			}
	
			VIFStaticDMA.AddUnpack(V4_32, 0, iTrisThisChunk * 9 + 2, 1);
	
			VIFStaticDMA.Add128(iTrisThisChunk * 9);
	
			VIFStaticDMA.Add128(
				GS_GIFTAG_BATCH(iTrisThisChunk * 3, // NLOOP
						1,	// EOP
						1,	// PRE
						// GS_PRIM(    PRIM, IIP, TME, FGE,  ABE         , AA1, FST, CTXT, FIX)
						GS_PRIM(GS_TRIANGLE,  1,   1,   0 , bTransparent ,   0,   0,    0,   0),	// PRIM
						GIF_FLG_PACKED,
						GS_BATCH_3(GIF_REG_ST, GIF_REG_RGBAQ, GIF_REG_XYZ2)));
	
			for(iTriangle = 0; iTriangle < iTrisThisChunk; iTriangle++)
			{
				temptri = m_pGroups[iCurrentGroup].ptris[iAbsoluteTri++];
	
				VIFStaticDMA.AddVector(Vector4(temptri.vert[0].texuv.x, temptri.vert[0].texuv.y, 1, 0));
				VIFStaticDMA.AddVector(Vector4(temptri.vert[0].normal.x, temptri.vert[0].normal.y, temptri.vert[0].normal.z, 0));
				VIFStaticDMA.AddVector(Vector4(temptri.vert[0].position.x, temptri.vert[0].position.y, temptri.vert[0].position.z, 1));
			
				VIFStaticDMA.AddVector(Vector4(temptri.vert[1].texuv.x, temptri.vert[1].texuv.y, 1, 0));
				VIFStaticDMA.AddVector(Vector4(temptri.vert[1].normal.x, temptri.vert[1].normal.y, temptri.vert[1].normal.z, 0));
				VIFStaticDMA.AddVector(Vector4(temptri.vert[1].position.x, temptri.vert[1].position.y, temptri.vert[1].position.z, 1));
	
				VIFStaticDMA.AddVector(Vector4(temptri.vert[2].texuv.x, temptri.vert[2].texuv.y, 1, 0));
				VIFStaticDMA.AddVector(Vector4(temptri.vert[2].normal.x, temptri.vert[2].normal.y, temptri.vert[2].normal.z, 0));
				VIFStaticDMA.AddVector(Vector4(temptri.vert[2].position.x, temptri.vert[2].position.y, temptri.vert[2].position.z, 1));
			}
			
			if(bFirst)
			{
				VIFStaticDMA.Add32(MSCALL(0));
				bFirst = false;
			}
			else
				VIFStaticDMA.Add32(MSCNT);
		}
		

		VIFStaticDMA.Add32(FLUSH);
		VIFStaticDMA.DMARet();
	}
}

void CMs3dModel::Render(void)
{	
	assert(m_bWorldLoaded && m_bDataLoaded);
	int i;
	
	// Wait for all to be idle in VU1 land
	// Before uploading the data
	VIFDynamicDMA.Add32(FLUSH);
	
	// Upload the matrices in the dynamic buffer
	// because they can change every frame.
	VIFDynamicDMA.AddUnpack(V4_32, 1, 20);
	VIFDynamicDMA.AddMatrix(m_WVP);
	VIFDynamicDMA.AddMatrix(m_World);
	VIFDynamicDMA.AddVector(Pipeline.GetCameraPos());
	VIFDynamicDMA.AddMatrix(Pipeline.GetLightDirs());
	VIFDynamicDMA.AddVector(Pipeline.GetLight1Vec());
	VIFDynamicDMA.AddVector(Pipeline.GetLight2Vec());
	VIFDynamicDMA.AddVector(Pipeline.GetLight3Vec());
	VIFDynamicDMA.AddMatrix(Pipeline.GetLightCols());
	
	for(i=0; i<m_iNumGroups;i++)
	{
		// Wait for all to be idle in VU1 land
		// Before uploading the data
		VIFDynamicDMA.Add32(FLUSH);
		
		// Call the packet with the static data that doesn't change per frame.
		VIFDynamicDMA.DMACall(m_pStaticAddrArr[i]);
	}
}
