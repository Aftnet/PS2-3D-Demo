/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:12 $
*
*/

// A nice manager to wrap up all the SPS2 memory handling etc. into a nice easy to use
// class.
#include "sps2wrap.h"
#include "PS2Defines.h"
#include "dma.h"

// We have to create this to use the singleton. (it calls the constructor
// which sets up the ms_Singleton member).
CSPS2Manager SPS2Manager_SingletonInit;

static int iScreenClear = 0;

// Constructor
CSPS2Manager::CSPS2Manager()
{
	// Write the disk cache to the HDD for safety
	printf("Writing cache to HDD\n");
	system("sync");
	
	// The array to keep track of how much memory is allocated in each page
	m_iaFreeMemInPage = 0;
}

// Destructor
CSPS2Manager::~CSPS2Manager()
{
	sps2UScreenShutdown();
	sps2Release(m_iSPS2Desc);

	// Free up all of the memory that was allocated in Initialise
	if(m_iaFreeMemInPage)
	{
		delete [] m_iaFreeMemInPage;
		m_iaFreeMemInPage = 0;
	}
}

// Set up SPS2, and allocate all of the SPS2 memory that will be
// used
void CSPS2Manager::Initialise(const int iAllocPages)
{
	// Keep track of how many 4K pages will be allocated
	m_iNumPages = iAllocPages;

	// Initialise SPS2
	m_iSPS2Desc = sps2Init();

	if(m_iSPS2Desc < 0)
	{
		fprintf(stderr, "Failed to initialise SPS2 library." \
				"Please check that the module is loaded.\n");
		exit(-1);
	}

	// Open the vector units
	int iVPU0; iVPU0 = open("/dev/ps2vpu0", O_RDWR);
	int iVPU1; iVPU1 = open("/dev/ps2vpu1", O_RDWR);

	// Allocate the memory that we will be using, and get the cached and
	// cached pointers.
	m_pMemCached = sps2Allocate(iAllocPages * 4096,
			SPS2_MAP_BLOCK_4K | SPS2_MAP_CACHED, m_iSPS2Desc);
	m_pMemUncached = sps2Remap(m_pMemCached, SPS2_MAP_UNCACHED, m_iSPS2Desc);

	// Set up an array to keep track of how much memory is free in each page
	m_iaFreeMemInPage = new int[m_iNumPages];

	for(int iPage = 0; iPage < m_iNumPages; iPage++)
	{
		m_iaFreeMemInPage[iPage] = 256;	// 4K == 256 Quad words
	}

	// Set up the screen
	sps2UScreenInit(0);
	
	// Enable EI DI instructions (this is needed for the screen shot code)
	_sps2SetEIDIEnabled(1, m_iSPS2Desc);
}

// Allocate some of the memory that we created in Initialise.
void CSPS2Manager::Allocate(CDMAMem & Memory, int iQuadWords)
{
	assert(iQuadWords <= 256 && "A CDMAMem class can only handle a maximum of 256 quadwords");

	for(int iPage = 0; iPage < m_iNumPages; iPage++)
	{
		// Find the first page with enough free memory in it.
		if(m_iaFreeMemInPage[iPage] >= iQuadWords)
		{
			// Set up the CDMAMem structure.
			// Get the first free quad word in the page.
			int FirstQW = 256 - m_iaFreeMemInPage[iPage];
			m_iaFreeMemInPage[iPage] -= iQuadWords;
			// Get the pointers.
			void * pCached = &((char *)m_pMemCached->pvStart)[iPage * 4096 + (FirstQW * 16)];
			void * pUncached = &((char *)m_pMemUncached->pvStart)[iPage * 4096 + (FirstQW * 16)];
			// Set the CDMAMem's member variables
			Memory.Set(iQuadWords, pCached, pUncached, sps2GetPhysicalAddress(pCached, m_pMemCached));
			return;	// No need to search anymore
		}
	}

	assert(!"Not enough free memory!");
}

// Initialise the screen clear. With this class we clear the screen much faster than SPS2 would
// usually. This puts all of the screen clear DMA data into our static DMA buffer. This means it is
// only ever added once (very fast), and is then simply CALLed from the dynamic DMA buffer.
// Notice that instead of just using one big sprite to clear the screen we use 20 long tall strips.
// This is because the GS is optimised for polygons less than or equal to 32 pixels wide, and it
// "chokes" on ones that are larger.
void CSPS2Manager::InitScreenClear(int R, int G, int B)
{
	int x0 = (2048 - (sps2UScreenGetWidth() >> 1)) << 4;
	int y0 = (2048 - (sps2UScreenGetHeight() >> 1)) << 4;
	int x1 = (2048 + (sps2UScreenGetWidth() >> 1)) << 4;
	int y1 = (2048 + (sps2UScreenGetHeight() >> 1)) << 4;

	// Get the address of the screen clear packet so we can CALL it
	// from the dynamic packet.
	iScreenClear = VIFStaticDMA.GetPointer();

	// Start the VIF direct mode.
	VIFStaticDMA.StartDirect();

	VIFStaticDMA.Add128(GS_GIFTAG_BATCH(4 + (20 * 2), 1, 0, 0, 
						GIF_FLG_PACKED, GS_BATCH_1(GIF_REG_A_D)));

	VIFStaticDMA.Add64(TEST_SET(0, 0, 0, 0, 0, 0, 1, 1));
	VIFStaticDMA.Add64(TEST_1);

	VIFStaticDMA.Add64(PRIM_SET(0x6, 0, 0, 0, 0, 0, 0, 0, 0));
	VIFStaticDMA.Add64(PRIM);

	VIFStaticDMA.Add64(RGBAQ_SET(R, G, B, 0x80, 0x3f800000));
	VIFStaticDMA.Add64(RGBAQ);

	for(int i = 0; i < 20; i++)
	{
		VIFStaticDMA.Add64(XYZ2_SET(x0, y0, 0));
		VIFStaticDMA.Add64(XYZ2);

		VIFStaticDMA.Add64(XYZ2_SET(x0 + (32 << 4), y1, 0));
		VIFStaticDMA.Add64(XYZ2);

		x0 += (32 << 4);
	}

	VIFStaticDMA.Add64(TEST_SET(0, 0, 0, 0, 0, 0, 1, 3));
	VIFStaticDMA.Add64(TEST_1);

	VIFStaticDMA.EndDirect();

	VIFStaticDMA.DMARet();
}

// This just DMACall's the screen clear packet that was set up in InitScreenClear
// (I'm sure you can see why this is so much faster than the sps2 screen clear, as the
//  CPU hardly has to do any work at all here).
void CSPS2Manager::BeginScene()
{
	VIFDynamicDMA.DMACall(iScreenClear);
}


int CSPS2Manager::FileExists(const char * const FileName)
{
	FILE * f;
	f = fopen(FileName, "rb");	// Try to open the file
	if (f == NULL) return 0; 	// File doesn't exist
	fclose (f);
	return 1;					// File exists
}	

void CSPS2Manager::ScreenShot(const int ImageType)
{
	char name[256];
	unsigned char * data;
	unsigned char tmp;
	unsigned short Width, Height;
	int BytesPerPixel = 4;
	
	// Current Screen width and height
	Width = (unsigned short)GetScreenWidth();
	Height = (unsigned short)GetScreenHeight();
	
	// Get an unused filename
	do
	{
		sprintf (name, "sshot_%02d.tga", m_SS_Number++);
	} while (FileExists(name));
	
	// Print the filename to the console with some stats
	if(ImageType == IMAGE_RGB)
		printf("Dumping Screen to File %s. Dimensions %d x %d - RGB\n", name, Width, Height);
	else if(ImageType == IMAGE_RGBA)
		printf("Dumping Screen to File %s. Dimensions %d x %d - RGBA\n", name, Width, Height);
	
	
	// Open the file for writing to	
	FILE * f = fopen(name, "wb");

	// Create a targa header for a 640x480, 32bit picture
	unsigned char header[18] = {
		0,				// Field 1 - Number of Bytes in Field 6
		0, 				// Field 2 - Colour Map Type 0 = no colour map
		2, 				// Field 3 - Image Type = Uncompressed true colour image
		0, 0, 0, 0, 0,	// Field 4 - Colour map specification
						// Field 5 - The remaining 10 bytes
		0, 0, 			// X origin
		0, 0, 			// Y origin
		0x80, 2, 		// Width 640
		0xe0, 1, 		// Height 480
		32, 			// Pixel colour depth
		8				// Number of alpha channel bits
	};
	
	//for(int i =0; i< 18; i++)printf("%x ", header[i]);printf("\n");
		
	//Change the width and height in the header to the correct dimensions
	unsigned char * pDimension = (unsigned char *)(&Width);
	//printf("%x %x\n", pDimension[0], pDimension[1]);
	header[12] = pDimension[0];
	header[13] = pDimension[1];
	
	pDimension = (unsigned char *)(&Height);
	header[14] = pDimension[0];
	header[15] = pDimension[1];
	
	// Change header and bytes per pixel if RGB
	if(ImageType == IMAGE_RGB)
	{
		header[16] = 24;
		header[17] = 0;
		BytesPerPixel = 3;
	}
	
	
	//for(int i =0; i< 18; i++)printf("%x ", header[i]);printf("\n");
	
	// write the header to the file	
	fwrite(header, 18, 1, f);
	
	// Get two pages of memory to work with
	// one page for the DMA chain, 
	CDMAMem DMAChainMem = VIFDynamicDMA.getTempPage(0);
	// One page to hold the frame buffer data
	CDMAMem SShotMem = VIFDynamicDMA.getTempPage(1);
	
	// Now Download the Frame buffer, from the bottom up
	for(int y = 0; y < Height; y++)
	{
		DownloadVram(DMAChainMem, SShotMem, 0, 10, 0, (Height - 1 - y), Width, 1);
		
		// convert RGBA to BGRA
		data = (unsigned char *)SShotMem.GetUncached();
		for(int x = 0; x < Width; x++)
		{
			tmp = data[0];
			data[0] = data[2];
			data[2] = tmp;
			
			// Write pixel to file
			fwrite(data, BytesPerPixel, 1, f);
			
			data += 4;
		}
	}
	
	// Close the file
	fclose(f);
}

void CSPS2Manager::DownloadVram(CDMAMem & DMAChainMem, CDMAMem & SShotMem, 
								int addr, int bufw, int x, int y, int w, int h)
{
	// Pointer the memory for building the DMA chain
	uint32 * pDmaMem32 = (uint32 *)DMAChainMem.GetUncached();	

	uint32 uSrcPtr = addr;
	uint32 uQSize = (w*h*4)/16; // assuming 32bit framebuffer

	// Setup transfer texture back to memory
	pDmaMem32[0] = 0; //NOP;
	pDmaMem32[1] = 0x06008000; //MSKPATH3(0x8000);
	pDmaMem32[2] = 0x13000000; //FLUSHA;
	pDmaMem32[3] = 0x50000006; //DIRECT(6);

	// Add GifTag Data
	// Get pointer to GIFTag position
	uint128 * pGIFTag = (uint128 *)(pDmaMem32 + 4);
	
	// Store the GIFTag - The Giftag for A+D mode
	*pGIFTag = GS_GIFTAG_BATCH(	5,							// NLOOP
	 							1,							// EOP
	 							0,							// PRE
	 							0,							// PRIM				
	 							GIF_FLG_PACKED,				// FLG
	 							GS_BATCH_1(	GIF_REG_A_D));	// BATCH
	 
	// Get uint64 pointer to write the A_D data
	// [0] and [1] are the GIFTag									
	uint64 * pDmaMem64 = (uint64 *) (pGIFTag);

	pDmaMem64[2] = (BITBLTBUF_SET( uSrcPtr, bufw, 0, 0, 0, 0 ));
	pDmaMem64[3] = (GIF_A_D_REG_BITBLTBUF);

	pDmaMem64[4] = (TRXPOS_SET(x, y, 0, 0, 0)); // SSAX, SSAY, DSAX, DSAY, DIR
	pDmaMem64[5] = (GIF_A_D_REG_TRXPOS);

	pDmaMem64[6] = (TRXREG_SET(w, h)); // RRW, RRh
	pDmaMem64[7] = (GIF_A_D_REG_TRXREG);

	pDmaMem64[8] = (0);
	pDmaMem64[9] = (GIF_A_D_REG_FINISH);

	pDmaMem64[10] = (TRXDIR_SET(1)); // XDIR
	pDmaMem64[11] = (GIF_A_D_REG_TRXDIR);

	uint32 prev_chcr = *EE_D1_CHCR;
	
	// *_GS_IMR fetches the current setting of interrupt mask register (imr)
	// DPUT_GS_IMR() sets the current value of imr
	// Mark interrupts from the FINISH Event (GS Page 154)
	uint32 prev_imr = *_GS_IMR;
	DPUT_GS_IMR( prev_imr | 0x0200 );
	
	// vif1 must be available
	if ((*EE_D1_CHCR & 0x0100) != 0)
	{
		printf("VIF1 is not available for the Screen Shot\n");
		exit(-1);
	}	
	
	// Enable generation of the FINISH Event (GS Page 145)
	DPUT_GS_CSR( 2 ); 
	
	// Disable interrupts so we don't lose our finish event
	asm __volatile__ ("di");

	
	// Make sure all cache is writen to main memory
	FlushCache();
	
	// Setup and start DMA transfer to VIF1
	*EE_D1_QWC = 0x7;
	*EE_D1_MADR = DMAChainMem.GetPhysicalAddr();
	//             DIR      MOD       ASP     TTE       TIE      STR
	*EE_D1_CHCR = (1<<0) | (0<<2) | (0<<4) | (0<<6) | (0<<7) | (1<<8);

	// Wait till memory load is complete
	asm __volatile__( " sync.l " );
	
	// Wait till DMA is complete (STR=0)
	while ( *EE_D1_CHCR & 0x0100 );

	// Wait for the GS to finish loading the registers (FINISH EVENT)
	//while ( ((*GS_CSR_OFF(SPS2_GS_REGISTERS_START)) & 2) == 0 ); 
	while ( (*_GS_CSR & 2) == 0 );
	
	// Re-enable interrupts
	asm __volatile__ ("ei");
	
	
	// START! The actual download to memory 
	// wait for VIF1-FIFO to empty and become idle 
	while(*EE_VIF1_STAT & 0x1f000003); 
	
	// Change the VIF1 direction to VIF1->Memory
	*((volatile uint32 *) EE_VIF1_STAT) = (1<<23);
	
	// Change direction of VIF1 FIF0 Local->Host
	DPUT_GS_BUSDIR( ( uint64 ) 0x00000001 );
	
	// Setup and start DMA transfer from VIF1
	*EE_D1_QWC = uQSize;
	*EE_D1_MADR = SShotMem.GetPhysicalAddr();
	//          DIR    MOD       ASP     TTE       TIE      STR
	*EE_D1_CHCR = 0 | (0<<2) | (0<<4) | (0<<6) | (0<<7) | (1<<8);

	// Wait till memory load is complete
	asm __volatile__( " sync.l " );

	// check if DMA is complete (STR=0)
	while ( *EE_D1_CHCR & 0x0100 );
	
	// Restore CHCR and wait for it to complete
	*EE_D1_CHCR = prev_chcr;
	asm __volatile__( " sync.l " );
	
	// wait for VIF1-FIFO to empty and become idle 
	while ( *EE_VIF1_STAT & 0x1f000003 ); 

	
	// Change the VIF1 and VIF1 FIFO Direction back to normal
	*((volatile uint32 *) EE_VIF1_STAT) = 0;
	DPUT_GS_BUSDIR( ( uint64 ) 0 );
	
	
	// restore the setting of IMR
	DPUT_GS_IMR( prev_imr);
	// set the FINISH event to default
	//dput_gs_csr( GS_EE_CSR_FINISH_M );
	DPUT_GS_CSR( 2 ); 
	
	
	
	// Re-enable path3 transfers
	static uint32 enable_path3[4] ALIGN_QW = {
	0x06000000, //MSKPATH3(0),
	0,
	0,
	0};

	DPUT_EE_VIF1_FIFO( *( uint128 * ) enable_path3 );

	// Make sure data is written to main memory
	FlushCache();
}

