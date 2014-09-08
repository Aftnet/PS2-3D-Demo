;/*
;* "PS2" Application Framework
;*
;* University of Abertay Dundee
;* May be used for educational purposed only
;*
;* Author - Dr Henry S Fortuna
;*
;* $Revision: 1.2 $
;* $Date: 2007/08/19 12:45:13 $
;*
;*/

; The static (or initialisation buffer) i.e. stuff that doesn't change for each
; time this code is called.
Scales			.assign 0
VWPTransform	.assign 1
WorldTrans		.assign 5 
CameraPos		.assign 9
LightDirs		.assign 10
Light1Vec		.assign 14
Light2Vec		.assign 15
Light3Vec		.assign 16
LightCols		.assign 17
MatAmbient		.assign 21
MatDiffuse		.assign 22
MatSpecular		.assign 23
MatEmissive		.assign 24
MatShininess	.assign 25

; The input buffer (relative the the start of one of the double buffers)
NumVerts		.assign 0 
GifPacket  		.assign	1 
UVStart			.assign 2
NormStart		.assign 3
VertStart		.assign 4

; The output buffer (relative the the start of one of the double buffers)
GifPacketOut	.assign 246
UVStartOut		.assign 247
NormStartOut	.assign 248
VertStartOut	.assign 249


; Note that we have 4 data buffers: InputBuffer0, OutputBuffer0, InputBuffer1, and OutputBuffer1.
; Each buffer is 248 quad words. The different buffers are selected by reading the current offset
; from xtop (which is swapped automatically by the PS2 after each MSCALL / MSCNT).


.include "vcl_sml.i"

.init_vf_all
.init_vi_all
.syntax new

.vu

--enter
--endenter

; The START or Init code, that is only called once per frame per model per group.
START:
	fcset		0x000000

	lq			fScales, Scales(vi00)
	lq			fVWPTransform[0], VWPTransform+0(vi00)
	lq			fVWPTransform[1], VWPTransform+1(vi00)
	lq			fVWPTransform[2], VWPTransform+2(vi00)
	lq			fVWPTransform[3], VWPTransform+3(vi00)
	lq			fWorldTrans[0], WorldTrans+0(vi00)
	lq			fWorldTrans[1], WorldTrans+1(vi00)
	lq			fWorldTrans[2], WorldTrans+2(vi00)
	lq			fWorldTrans[3], WorldTrans+3(vi00)
	lq			fCameraPos, CameraPos(vi00)
	lq			fLightDirs[0], LightDirs+0(vi00)
	lq			fLightDirs[1], LightDirs+1(vi00)
	lq			fLightDirs[2], LightDirs+2(vi00)
	lq			fLight1Vec, Light1Vec(vi00)
	lq			fLight2Vec, Light2Vec(vi00)
	lq			fLight3Vec, Light3Vec(vi00)
	lq			fLightCols[0], LightCols+0(vi00)
	lq			fLightCols[1], LightCols+1(vi00)
	lq			fLightCols[2], LightCols+2(vi00)
	lq			fLightCols[3], LightCols+3(vi00)
	lq			fMatAmbient, MatAmbient(vi00)
	lq			fMatDiffuse, MatDiffuse(vi00)
	lq			fMatSpecular, MatSpecular(vi00)
	lq			fMatEmissive, MatEmissive(vi00)
	

; This begin code is called once per batch
begin:
	xtop		iDBOffset		; Load the address of the current buffer (will either be QW 16 or QW 520)
	ilw.x		iNumVerts, NumVerts(iDBOffset)
	iadd		iNumVerts, iNumVerts, iDBOffset
	iadd		Counter, vi00, iDBOffset
		

loop:
	lq			Vert, VertStart(Counter)				; Geometry calculations: Load the vertex from the input buffer
	mul         acc,  fVWPTransform[0],  Vert[x]		; Transform it
    madd        acc,  fVWPTransform[1],  Vert[y]
    madd        acc,  fVWPTransform[2],  Vert[z]
    madd        Vert, fVWPTransform[3],  Vert[w]
    clipw.xyz	Vert, Vert								; Clip it
	fcand		vi01, 0x3FFFF
	iaddiu		iADC, vi01, 0x7FFF
	ilw.w		iNoDraw, UVStart(Counter)				; Load the iNoDraw flag. If true we should set the ADC bit so the vert isn't drawn
	iadd		iADC, iADC, iNoDraw
	isw.w		iADC, VertStartOut(Counter)
	div         q,    vf00[w], Vert[w]			
	lq			UV,   UVStart(Counter)					; Texturing calculations: Handle the tex-coords
	mul			UV,   UV, q
	sq			UV,   UVStartOut(Counter)
	mul.xyz     Vert, Vert, q							; Scale the final vertex to fit to the screen.
	mula.xyz	acc, fScales, vf00[w]
	madd.xyz	Vert, Vert, fScales
	ftoi4.xyz	Vert, Vert
	sq.xyz		Vert, VertStartOut(Counter)				; And store in the output buffer
	adda.xyzw	acc, fMatEmissive, vf00[x]				; Lighting Calculations: calculate base color (emissive + mat_diffuse * light)
	madd.xyzw	FinalColor, fMatAmbient, fLightCols[3]
	lq.xyz		Norm, NormStart(Counter)				; Diffuse lighting Calculations: load the normal
	mula.xyz    acc,  fWorldTrans[0],  Norm[x]			; Transform by the rotation part of the world matrix
    madd.xyz    acc,  fWorldTrans[1],  Norm[y]
    madd.xyz    Norm, fWorldTrans[2],  Norm[z]			; Norm is now in world space
    mula.xyz	acc, fLightDirs[0], Norm[x]				; "Transform" the normal by the light direction matrix
    madd.xyz	acc, fLightDirs[1], Norm[y]				; This has the effect of outputting a vector with all
    madd.xyz	fIntensities, fLightDirs[2], Norm[z]	; four intensities, one for each light.
    mini.xyz	fIntensities, fIntensities, vf00[w]		; Clamp the intensity to 0..1
    max.xyz		fIntensities, fIntensities, vf00[x]
	mula.xyz	acc, fLightCols[0], fIntensities[x]		; Transform the intensities by the light colour matrix
	madd.xyz	acc, fLightCols[1], fIntensities[y]		; This gives the final total directional light colour
	madd.xyz	fIntensities, fLightCols[2], fIntensities[z]
	add.xyz		acc, FinalColor, vf00					; Add diffuse color * mat_diffuse to previous color
	madd.xyz	FinalColor, fIntensities, fMatDiffuse
	lq			Vert, VertStart(Counter)				; Specular calculations: load local vertex position
	mul         acc,  fWorldTrans[0],  Vert[x]			; Transform it (again) to world coordinates 
    madd        acc,  fWorldTrans[1],  Vert[y]
    madd        acc,  fWorldTrans[2],  Vert[z]
    madd        Vert, fWorldTrans[3],  Vert[w]
	sub			fVertCam, fCameraPos, Vert				; Find the vertex to camera vector
	add.w 		fVertCam, vf00, vf00[x]
	VectorNormalizeXYZ	fVertCam,fVertCam				; Normalize it
	add			fTmp, fVertCam, fLight1Vec				; Add vertex to camera vector to light direction 1 to find Half-vector
	VectorNormalizeXYZ	fTmp,fTmp						; Normalize it
	VectorDotProduct  fIntensities,fTmp,Norm			; Calculate dot product of half vector and normal (stored in the x component of the output vector)
	add			fTmp, fVertCam, fLight2Vec				; Add vertex to camera vector to light direction 2 to find Half-vector
	VectorNormalizeXYZ	fTmp,fTmp						; Normalize it
	VectorDotProduct  fTmp,fTmp,Norm					; Calculate dot product of half vector and normal (stored in the x component of the output vector)
	add.y		fIntensities, vf00, fTmp[x]				; Transfer to y component of intensities vector
	add			fTmp, fVertCam, fLight3Vec				; Add vertex to camera vector to light direction 3 to find Half-vector
	VectorNormalizeXYZ	fTmp,fTmp						; Normalize it
	VectorDotProduct  fTmp,fTmp,Norm					; Calculate dot product of half vector and normal (stored in the x component of the output vector)
	add.z		fIntensities, vf00, fTmp[x]				; Transfer to z component of intensities vector
	mini.xyz	fIntensities, fIntensities, vf00[w]		; Clamp the intensities to 0..1
    max.xyz		fIntensities, fIntensities, vf00[x]
	ilw.x		iShinCtr, MatShininess(vi00)			; Raise to closest power of 2 to shininess index
	ibeq		iShinCtr, vi00, risend
rise:
	mul.xyz		fIntensities, fIntensities, fIntensities
	isubiu		iShinCtr, iShinCtr, 1
	ibne		iShinCtr, vi00, rise
risend:	
	mini.xyz	fIntensities, fIntensities, vf00[w]		; Clamp the intensities to 0..1
    max.xyz		fIntensities, fIntensities, vf00[x]
	mul.xyz		acc, fLightCols[0], fIntensities[x]		; Transform the intensities by the light colour matrix
    madd.xyz	acc, fLightCols[1], fIntensities[y]		; This gives the final total specular light colour
    madd.xyz	fIntensities, fLightCols[2], fIntensities[z]
	add.xyzw	acc, FinalColor, vf00[x]				; Add specular color * mat_specular to previous color
	madd.xyz	FinalColor, fIntensities, fMatSpecular
	mini.xyz	FinalColor, FinalColor, vf00[w]			; Clamp the final color to 0..1
    max.xyz		FinalColor, FinalColor, vf00[x]
    loi			128										; Load 128 
	muli		FinalColor, FinalColor, i				; Multiply the normalized color components by 128 (max reasonable value for display)
	addi.w		FinalColor, vf00, i						; Put 128 into the alpha value
    ftoi0		FinalColor, FinalColor
	sq			FinalColor, NormStartOut(Counter)		; And write to the output buffer
	iaddiu		Counter, Counter, 3
	ibne		Counter, iNumVerts, loop				; Loop until all of the verts in this batch are done.
	iaddiu		iKick, iDBOffset, GifPacketOut
	lq			GP, GifPacket(iDBOffset)				; Copy the GIFTag to the output buffer
	sq			GP, GifPacketOut(iDBOffset)
	xgkick		iKick									; and render!
	
--cont
														; --cont is like end, but it really means pause, as this is where the code
														; will pick up from when MSCNT is called.
	b			begin									; Which will make it hit this code which takes it back to the start, but
														; skips the initialisation which we don't want done twice.

--exit
--endexit
