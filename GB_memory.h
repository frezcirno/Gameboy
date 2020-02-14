#pragma once
#include "GB_common.h"
#include <stdio.h>

extern const GB_BYTE GB_bios[0x100];

typedef GB_BYTE (*readByteType) (struct GB_MEMORY*, GB_WORD);

typedef void (*writeByteType) (struct GB_MEMORY*, GB_WORD, GB_BYTE);

typedef struct GB_CARTHEADER {
	char title[16 + 1];  // 0134-0142
	int romSize;
	int ramSize;
	GB_BYTE cartType;
	int mbcType;
	//GB_BYTE isCGB;    // 0143 $80=CGB other=not CGB
	//GB_BYTE isSGB;    // 0146 $03=SGB $00=GB
	//int licCode;
	//int dstCode;     // 014A $0=JP $1=not JP
	//int oldLicCode;  // 014B $33=to 0144/5
}GB_CARTHEADER;

typedef struct GB_MEMORY
{
	struct GB_GAMEBOY* parent;
	int inBios;
	GB_CARTHEADER cartHeader;
	GB_BYTE cartType;  // 0x0147 卡带类型

	const GB_BYTE* rom;
	GB_BYTE* eram;

	struct {
		readByteType readByte;
		writeByteType writeByte;
	} mbc;

	GB_BYTE wram[0x2000];
	GB_BYTE zram[0x80];
	GB_BYTE unimpledIoRegs[0xE0]; // 0xFEA0-0xFF80

	/* I/O Registers */
	GB_BYTE joypadIn; // 0xFF00 bit 4 & bit 5 (Power +) 取值 {0, 1}
	GB_BYTE joypadOut[2];	// 0xFF00 bit 0-3 (Power -) 低四位有效

	GB_WORD __clocks;  // 高8位是divider
	//GB_BYTE divider;   // 0xFF04 定频(16,384Hz)时钟 范围0-255
	GB_BYTE tima;   // 0xFF05 可调频可调起点时钟 范围modulo-255 溢出时会产生0x50号中断
	GB_BYTE tma;    // 0xFF06 counter时钟的起点
	GB_BYTE tac;   // 0xFF07 counter时钟设置, 第0-1位控制速度, 第2位控制开关
	/*
		时钟周期(1 clk): 4,194,304Hz
		机器周期(4 clk): 1,048,576Hz <- insnCycles
		CPU内部时钟(16 clk): 262,144Hz
		Divider(256 clk): 16,384Hz
		Counter: 00(1024clk): 4,096Hz
				 01(16clk): 262,144Hz
				 02(64clk): 65,536Hz
				 03(256clk): 16,384Hz
	*/

	GB_BYTE intFlag; // 0xFF0F 中断 bit 0: VBlank; bit 1: LCDC; bit 2: TIMER
	/*
		Bit 4: Transition from High to Low of Pin number P10-P13.
		Bit 3: Serial I/O transfer complete
	*/
	GB_BYTE intEnMask; // 0xFFFF 中断开关 bit 0: VBlank; bit 1: LCDC; bit 2: TIMER

} GB_MEMORY;

int memory_init(GB_MEMORY* mem);
void memory_free(GB_MEMORY* mem);

GB_BYTE readByte(GB_MEMORY* mem, GB_WORD addr);

GB_WORD readWord(GB_MEMORY* mem, GB_WORD addr);

void writeByte(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val);

void writeWord(GB_MEMORY* mem, GB_WORD addr, GB_WORD val);

int loadCart(GB_MEMORY* mem, FILE* fp);

int readCartHeader(FILE * fp, GB_CARTHEADER* header);

int timer_step(GB_MEMORY* mem, int cycles);

