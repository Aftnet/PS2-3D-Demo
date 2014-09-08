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

#ifndef _TIMER_H_
#define _TIMER_H_


// Timer 3 uses the HBLANK to count 
// (its the only default timer that is slow enough 
// for us to sample in order to calculate the FPS)


class CTimer
{
public:
	CTimer();
	~CTimer();
	
	unsigned int 	GetTimeDelta(void);
	void 			PrimeTimer(void);
	float		 	GetFPS(void);
	
private:
	
	unsigned int m_LastTime;
	unsigned int m_Time;
	unsigned int m_Change;
	unsigned int m_VideoMode;
};


#endif

