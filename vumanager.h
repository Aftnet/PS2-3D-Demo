/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:14 $
*
*/

#ifndef __VUMANAGER_H__
#define __VUMANAGER_H__

#include "dma.h"

class CVU1MicroProgram
{
public:
	// pCode is the start of the VU code, and pCodeEnd is the last DWord.
	// Offset can be used to upload the code further into VU1 micro memory.
	CVU1MicroProgram(uint64 * pCode, uint64 * pCodeEnd, int Offset = 0);
	~CVU1MicroProgram(){}

	void Upload();	// Send the program to VU1 memory

	// Get the code size in double words (note that there are 2048 QWs in VU1 micro memory)
	// So you can have quite a few microprograms loaded at once.
	int GetCodeSizeDW()
	{ return m_iOffset; }

private:
	int m_iOffset;
	int m_pUploadPtr;
	int m_iSize;
};

#endif