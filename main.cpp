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
#include <assert.h>
#include <memory.h>
#include <signal.h>
#include "PS2Defines.h"
#include "ps2maths.h"
#include "ps2matrix4x4.h"
#include "pad.h"
#include "sps2wrap.h"
#include "dma.h"
#include "texture.h"
#include "font.h"
#include "pipeline.h"
#include "vumanager.h"
#include "timer.h"
#include "ms3dmodel.h"

/*
	Look at the start of the vcl code to get the positions
	and layout of the packet data in VU data memrory.
	Remember that the data sent to the VU must be in 
	the specified format of the PS2 WILL CRASH

*/


// These two external function pointers are exported by the VU code
// and we can now use them as pointers to the compiled VU data
extern "C" uint64 VU_vu1code_start;
extern "C" uint64 VU_vu1code_end;


bool g_bLoop = true;

void sig_handle(int sig)
{
	g_bLoop = false;
}


int main(void)
{	
	// Make sure these four lines are put first since some of the 
	// other classes need the managers to be initialised so they 
	// can pick up the correct data.	
	SPS2Manager.Initialise(4096);	// 4096 * 4K Pages = 16MB Total
	VIFStaticDMA.Initialise(3072);	// 1536 * 4K Pages = 6MB Static DMA
	VIFDynamicDMA.Initialise(256);	// 256 * 4K Pages * 2 Buffers =
									// 2MB Dynamic DMA
	Pipeline.Initialise();			// Initialise graphics pipline class
	
	
	// Initialise Lighting
	// Three lights and Ambient light
	//							Direction vector				  Colour	
	Pipeline.SetLight1(Vector4( 1.0f, 0.0f, 0.0f, 0.0f), Vector4(1.0f,1.0f,1.0f,0.0f));
	Pipeline.SetLight2(Vector4(-1.0f,-0.2f, 0.0f, 0.0f), Vector4(1.0f,1.0f,1.0f,0.0f));
	Pipeline.SetLight3(Vector4( 0.0f, 0.0f,-1.0f, 0.0f), Vector4(0.0f,0.0f,0.0f,0.0f));
	//                            Colour
	Pipeline.SetAmbient(Vector4(1.0f,1.0f,1.0f,0.0f));
	
	// Model to render
	CMs3dModel Model;
	CTexture ModelTex;
	
	// Load the model itself.
	if(!Model.LoadModelData("alien.ms3d", false, false))
		g_bLoop = false;
	
	//Useless here, creates an array with all the model's triangle data (vertex positions and triangle normal). It could form the basis for collision detection with an arbitrary world 3D mesh.
	//triangle *Array;
	//Array = Model.CreateTriInfoArray();
	//Model.DeleteTriInfo();
	//Useless here, gets number of triangles the model is made of
	//int numtrismodel = Model.GetTotalNumTriangles();

	
	// Now load the model's texture.
	if(!ModelTex.LoadBitmap("whitetex.bmp"))
	{
		printf("Can't load Texture\n");
		g_bLoop = false;
	}
	
	// Performance timer - call after SPS2Manager.Initialise()
	CTimer Timer;
	
	// Initialise control pad 0
	if(!pad_init(PAD_0, PAD_INIT_LOCK | PAD_INIT_ANALOGUE | PAD_INIT_PRESSURE))
	{
		printf("Failed to initialise control pad\n");
		pad_cleanup(PAD_0);
		exit(0);
	}	
	
	// Initialise the VU1 manager class
	CVU1MicroProgram VU1MicroCode(&VU_vu1code_start, &VU_vu1code_end);
	
	// Upload the microcode
	VU1MicroCode.Upload();
	

	// Register our signal function for every possible signal (i.e. ctrl + c)
	for(int sig=0; sig<128; sig++)
		signal(sig, sig_handle);
		
		
	// Set up the DMA packet to clear the screen. We want to clear to blue.
	SPS2Manager.InitScreenClear(0, 0x25, 0x50);
	
	// Load the texture for the cube
	CTexture Test;
	if(!Test.LoadBitmap("test.bmp"))
	{
		printf("Can't load Texture\n");
		g_bLoop = false;
	}
	
	// Load the font bitmap and data
	CFont Font;
	CTexture FontTex;
	if(!Font.Load("font.dat", true))
	{
		g_bLoop = false;
		printf("Can't load font.dat\n");
	}
	if(!FontTex.LoadBitmap("font.bmp", false, true))
	{
		printf("Can't load font.bmp\n");
		g_bLoop = false;
	}

	// Upload these once to VRAM since
	// they are not going to change
	//Test.Upload(TEXBUF480);
	FontTex.Upload(TEXBUF496);
	
	//Pipeline.PositionCamera(Vector4(0.0f, 35.0f, 80.0f, 1.0f), 0.0f, 0.0f);
	Pipeline.PositionCamera(Vector4(0.0f, 55.0f, 80.0f, 1.0f), Vector4(0.0f, 40.0f, 0.0f, 1.0f));


	int Frame = 0;
	float fps = 0.0f;
		
	// The main Render loop
	while(g_bLoop)
	{
		//start the time
  		Timer.PrimeTimer();
  		

  	  		
		VIFDynamicDMA.Fire();
		
		pad_update(PAD_0);
		
		// Check for exit condition
		if((pad[0].buttons & PAD_START)&&(pad[0].buttons & PAD_SELECT)) g_bLoop = false;
		
		// Set up a world matrix that rotates the cube about it's own axis, and then again
		// around the origin.
		static float sfRotOrigin = 0.0f;
		static float sfRotLocal = 0.0f;
		sfRotOrigin += pad[0].pressures[PAD_PLEFT] * 0.1f;
		sfRotOrigin -= pad[0].pressures[PAD_PRIGHT] * 0.1f;
		sfRotLocal += 0.03f;

		Matrix4x4 matWorld, matTrans, matRotLocal, matRotOrigin;
		matTrans.Translation(0, 0, -8.0f);
		matRotOrigin.RotationY(sfRotOrigin);
		matRotLocal.RotationY(sfRotLocal);

		matWorld = matRotLocal * matTrans * matRotOrigin;
		
		// Get any camera movements
		// Get any requested translations
		float Strafe =   pad[0].axes[PAD_AXIS_LX];
		float Advance =  pad[0].axes[PAD_AXIS_LY];
		float UpDown =   (pad[0].pressures[PAD_PL1] - pad[0].pressures[PAD_PL2]);
		
		// Get requested rotations
		float YRot = pad[0].axes[PAD_AXIS_RY];
		float XRot = pad[0].axes[PAD_AXIS_RX];
				
		// Reset camera to default position if requested
		if(pad[0].buttons & PAD_R3)
		{
			Pipeline.PositionCamera(Vector4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f, 0.0f);
		}
		
		// Update the Camera and viewProjection matrices
		Pipeline.Update(Strafe, Advance, UpDown, YRot, XRot);
			
		SPS2Manager.BeginScene();
		
		// Select the model texture
		ModelTex.Upload(TEXBUF480);
		ModelTex.Select();		
		
		// Render Model
		matWorld(3,1) = 60.0f;
		Model.SetWorldMatrix(matWorld);
		Model.Render();
		
		
			
		// Select the Font texture
		//FontTex.Upload(TEXBUF496);	
		FontTex.Select();
		
		// Render some text
		Font.printfL(  	-300, -240, 127, 127, 127, 127, 
						"Camera Position (x, y, z) = (%3.1f, %3.1f, %3.1f)", 
						Pipeline.GetCameraX(), Pipeline.GetCameraY(), Pipeline.GetCameraZ());
		Font.printfL(  	-300, -210, 127, 127, 127, 127, 
						"Camera Rotation in Degrees (XRot, YRot) = (%3.1f, %3.1f)", 
						RadToDeg(Pipeline.GetCameraXRot()), RadToDeg(Pipeline.GetCameraYRot()));
		Font.printfC(  200, -240, 127, 127, 127, 127, "Frame: %d\nFPS: %.1f" , Frame++, fps);
				
		//calculate the current frame rate in frames per second	
		fps = Timer.GetFPS();
		
		SPS2Manager.EndScene();	
		
		if(pad[0].pressed & PAD_TRI)SPS2Manager.ScreenShot();		
	}

	pad_cleanup(PAD_0);
	
	return 0;
}

