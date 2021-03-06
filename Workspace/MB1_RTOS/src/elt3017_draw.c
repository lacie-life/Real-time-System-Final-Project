/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
//#include "xil_io.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "math.h"
#include "sleep.h"
#include "elt3017_draw.h"
#include "mb1.h"
#include "stdlib.h"
#include "time.h"
/*****************************************************************************/
/******************* Macros and Constants Definitions ************************/
/*****************************************************************************/
#define MIN(x, y)				(x < y ? x : y)
#define MAX(x, y) 				(x > y ? x : y)
#define DIV_ROUND_UP(x,y)		(((x) + (y) - 1) / (y))
#define DIV_ROUND_CLOSEST(x, y)	(unsigned long)(((double)x / y) + 0.5)
#define CLAMP(val, min, max)	(val < min ? min : (val > max ? max :val))
#define ABS(x)					(x < 0 ? -x : x)
const u32 frame_base[3] = {VIDEO_BASEADDR,VIDEO_BASEADDR2,VIDEO_BASEADDR3};
time_t t;

#define edgeWidth 2
#define hole 20
#define edgeNumber 2
#define widthNumber 10
#define heightNumber 10 //height of number / 2


enum detailedTimingElement
{
	PIXEL_CLOCK,
	H_ACTIVE_TIME,
	H_BLANKING_TIME,
	H_SYNC_OFFSET,
	H_SYNC_WIDTH_PULSE,
	V_ACTIVE_TIME,
	V_BLANKING_TIME,
	V_SYNC_OFFSET,
	V_SYNC_WIDTH_PULSE
};

static const unsigned long detailedTiming[7][9] =
{
	{25180000, 640, 144, 16, 96, 480, 29, 10, 2},
	{40000000, 800, 256, 40, 128, 600, 28, 1, 4},
	{65000000, 1024, 320, 136, 24, 768, 38, 3, 6},
	{74250000, 1280, 370, 110, 40, 720, 30, 5, 5},
	{84750000, 1360, 416, 136, 72, 768, 30, 3, 5},
	{108000000, 1600, 400, 32, 48, 900, 12, 3, 6},
	{148500000, 1920, 280, 44, 88, 1080, 45, 4, 5}
};
#ifndef USE_MEMCPY
void DrawARect(int fidx, char resolution,unsigned long pixel, int w, int h, u32 val)
{
	unsigned long pindex = pixel;
	unsigned short horizontalActiveTime = detailedTiming[resolution][H_ACTIVE_TIME];
	for (int i=0; i<h;i++)
	{
		for (int j=0; j <w; j++)
		{
			pindex++;
			Xil_Out32((frame_base[fidx]+(pindex<<2)), (val & 0xffffff));
		}
		pindex=pindex+horizontalActiveTime-w;
	}

	Xil_DCacheFlush();
}
#else
#define DrawARect RENAME(DrawARectMemcpy)

void DrawARectMemcpy(int fidx, char resolution,unsigned long pixel, int w, int h, u32 val){
	int i;
	u32 pindex = (u32) (pixel*4)+ frame_base[fidx];
	unsigned short horizontalActiveTime = detailedTiming[(int) resolution][(int) H_ACTIVE_TIME];
	u32 * line;
	line = malloc(w*(4));
	for (i = 0; i < w; i++)
	{
	  *(u32*)(line + i) = val;
	}
	for (i = 0; i < h;i+=2)
	{
	  memcpy((void *)pindex, line, w*(4) );
	  pindex=pindex+horizontalActiveTime*(8);
	}
	free(line);
}
void DrawARectMemcpy_2(int fidx, char resolution,unsigned long pixel, int w, int h, u32 val){
	int i;
	u32 pindex = (u32) (pixel*4)+ frame_base[fidx];
	unsigned short horizontalActiveTime = detailedTiming[(int) resolution][(int) H_ACTIVE_TIME];
	u32 * line;
	line = malloc(w*(4));
	for (i = 0; i < w; i++)
	{
	  *(u32*)(line + i) = val;
	}
	for (i = 0; i < h;i++)
	{
	  memcpy((void *)pindex, line, w*(4) );
	  pindex=pindex+horizontalActiveTime*(4);
	}
	free(line);
}
void DrawARectMemset(int fidx, char resolution,unsigned long pixel, int w, int h, u32 val){
	int i;
	u32 pindex = (u32) (pixel*4)+ frame_base[fidx];
	unsigned short horizontalActiveTime = detailedTiming[(int) resolution][(int) H_ACTIVE_TIME];
	for (i = 0; i < h; i++)
	{
	  memset((void *)pindex, val, w*(4) );
	  pindex=pindex+horizontalActiveTime*(4);	}
}
#endif

void MoveDino(int fidx, unsigned long * pixel, int d, int movey){
	if (movey > 0) {
		DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel, d, movey, 0x000000);
		*pixel = *pixel + movey*640;
		DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel+(d-movey)*640, d, movey, SQR_COLOR);
	} else if (movey < 0) {
		*pixel = *pixel + movey*640;
		DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel, d, -1*movey, SQR_COLOR);
		DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel+d*640, d, -1*movey, 0x000000);
	}
}
void MoveTree(int fidx, unsigned long * x, int d, int speed){
	unsigned long pixel  = ToPindex(*x,(75));
	//srand(time(NULL));
	
	unsigned long pixel1 = ToPindex(*x,(75+d+50));

	if (*x < 640) {
		if (*x < 210) { // close to the end, disappear
			*x = 640;
			DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel, 11+speed+TreeWidth,d , 0x000000);
			DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel1, 11+speed+TreeWidth,176-(d+50) , 0x000000);
		} else {
			DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel+speed, TreeWidth , d, 0x000000);
			DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel, TreeWidth, d, 0x00ff12);
			DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel1+speed, TreeWidth , 176-(d+50), 0x000000);
			DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel1, TreeWidth, 176-(d+50), 0x00ff12);

			*x=*x-speed;
		}
	} else {*x = *x-speed;}
}

void MoveDinoUpDown(int fidx, unsigned long * pixel, int d, int movey){
	
	if (movey > 0) {
			DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel, d, d, 0x000000);
			*pixel = *pixel + movey*640;
			DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel, d, d, SQR_COLOR);
	}
	else if (movey < 0) {
		DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel, d, d, 0x000000);
		*pixel = *pixel + movey*640;
		DrawARectMemcpy_2(fidx, RESOLUTION_640x480, *pixel, d, d, SQR_COLOR);

		}
}
void scoreNumber(int number, int xPosition, int yPosition)
{
	unsigned int pixel = ToPindex(xPosition, yPosition);
	int value;
	int valueArray[10] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111};
	value = valueArray[number];
	int fidx = 0;
	//a
	if (value & 0x01)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + edgeNumber, widthNumber, edgeNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + edgeNumber, widthNumber, edgeNumber, 0x000000);
	}

	//b
	if (value & 0x02)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(widthNumber + edgeNumber,edgeNumber), edgeNumber, heightNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(widthNumber + edgeNumber,edgeNumber), edgeNumber, heightNumber, 0x000000);
	}

	//c
	if (value & 0x04)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(widthNumber + edgeNumber, (heightNumber+edgeNumber*2)), edgeNumber, heightNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(widthNumber + edgeNumber, (heightNumber+edgeNumber*2)), edgeNumber, heightNumber, 0x000000);
	}

	//d
	if (value & 0x08)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(edgeNumber, (edgeNumber+heightNumber+edgeNumber+heightNumber)), widthNumber, edgeNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(edgeNumber, (heightNumber * 2+edgeNumber*2)), widthNumber, edgeNumber, 0x000000);
	}

	//e
	if (value & 0x10)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(0, (heightNumber+edgeNumber*2)), edgeNumber, heightNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(0, (heightNumber+edgeNumber*2)), edgeNumber, heightNumber, 0x000000);
	}

	//f
	if (value & 0x20)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(0,edgeNumber), edgeNumber, heightNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(0,edgeNumber), edgeNumber, heightNumber, 0x000000);
	}

	//g
	if (value & 0x40)
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(edgeNumber, (edgeNumber+ heightNumber)), widthNumber, edgeNumber, 0xff0000);
	else
	{
		DrawARectMemcpy(fidx, RESOLUTION_640x480, pixel + ToPindex(edgeNumber, (heightNumber+edgeNumber)), widthNumber, edgeNumber, 0x000000);
	}
}

