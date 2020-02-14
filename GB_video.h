#pragma once
#include <SDL.h>
#include "GB_common.h"

typedef Uint32 GB_COLOR;

typedef struct GB_VIDEO {
	struct GB_GAMEBOY* parent;
	SDL_Surface* surface;
	SDL_Window* window;

	GB_BYTE mode, modeClock;
	GB_BYTE vram[0x2000]; // 0x8000-(tile set)-0x9800-(tile map)-0xA000
	GB_BYTE oam[160]; // OAM 40 x 4 Byte

	struct GB_OAM {
		GB_BYTE y; //Byte 0 Spirit Pos RealY = y - 16
		GB_BYTE x; //Byte 1 Spirit Pos RealX = x - 8
		GB_BYTE tileNo; //Byte 2 value from 0-255
		GB_BYTE prio;  //Byte 3 Bit 7 -1, 0
		GB_BYTE yflip; //Byte 3 Bit 6
		GB_BYTE xflip; //Byte 3 Bit 5
		GB_BYTE palNo; //Byte 3 Bit 4
		GB_BYTE padding;
	} objData[40];

	GB_BYTE tileData[384][8][8]; // Tile set

	GB_BYTE switchBg;  // 0xFF40 bit 0
	GB_BYTE switchObj; // 0xFF40 bit 1
	GB_BYTE switchWin; // 0xFF40 bit 5
	GB_BYTE switchAll; // 0xFF40 bit 7

	GB_BYTE objSize;   // 0xFF40 bit 2  8x8/8x16
	GB_BYTE bgMap; // 0xFF40 bit 3 Tile map
	GB_BYTE winMap;// 0xFF40 bit 6 Tile map
					//Tile map #0: [vram+0x1800, vram+0x1C00]32x32Byte=0x400
					//Tile map #1: [vram+0x1C00, vram+0x2000]
	GB_BYTE tileSet; // 0xFF40 bit 4  Tile set 16Bytex256=0x1000Byte (0x800 Overlap)
					//Tile set #0: [vram+0x800(-128), vram+0x1000(0), vram+0x1800(127)]
					//Tile set #1: [vram(0), vram+0x1000(256)], tile set #1

	GB_BYTE lcdIntEn;  // 0xFF41 STAT LCD bit 3-6  mode 0, mode 1, mode 2, lyc
	GB_BYTE scx, scy;   // 0xFF43, 0xFF42 Χ 0-255, 0-255
	GB_BYTE line;       // 0xFF44  
	GB_BYTE lyc;		// 0xFF45 line == lyc  STAT(0xFF41)
	GB_COLOR bgPal[4], objPals[2][4];  // 0xFF47, 0xFF48, 0xFF49
	GB_BYTE winX, winY; // 0xFF4A, 0xFF4B  [0,143], [0,166]
} GB_VIDEO;

int video_init(GB_VIDEO* gpu);

void video_free(GB_VIDEO* gpu);

int video_step(GB_VIDEO* gpu, int cycles);

int video_renderLine(GB_VIDEO* gpu);

int video_updateTile(GB_VIDEO* gpu, GB_WORD addr, GB_BYTE val);
int video_updateObj(GB_VIDEO* gpu, GB_WORD addr, GB_BYTE val);

int video_reset(GB_VIDEO* gpu);

GB_BYTE video_readByte(GB_VIDEO* gpu, GB_WORD addr);

void video_writeByte(GB_VIDEO* gpu, GB_WORD addr, GB_BYTE val);
