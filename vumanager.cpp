/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:13 $
*
*/

#include "vumanager.h"

CVU1MicroProgram::CVU1MicroProgram(uint64 * pCodeStart, uint64 * pCodeEnd, int Offset)
{
	// Create a static DMA segment that has an MPG vifcode followed by the
	// actual microcode data.

	m_pUploadPtr = VIFStaticDMA.GetPointer();
	m_iSize = pCodeEnd - pCodeStart;
	m_iOffset = Offset;

	VIFStaticDMA.StartMPG(Offset);

	for(int i = 0; i < m_iSize; i++)
	{
		VIFStaticDMA.AddMPG(pCodeStart[i]);
	}

	VIFStaticDMA.EndMPG();
	VIFStaticDMA.DMARet();
}

void CVU1MicroProgram::Upload()
{
	VIFDynamicDMA.Add32(FLUSHE);
	VIFDynamicDMA.DMACall(m_pUploadPtr);
}