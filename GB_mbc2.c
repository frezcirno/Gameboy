#include "GB_memory.h"

static GB_BYTE switchEram = 1;  // 0x0000-0x2000 拓展ram开关(W)
static GB_BYTE romBank = 1;   // 0x2000-0x4000 Switch between 0-15 (value 0 is seen as 1)
static GB_BYTE ram[256];

GB_BYTE readByteMbc2(GB_MEMORY* mem, GB_WORD addr) {
	if (addr >= 0x4000 && addr < 0x8000) {
		return mem->rom[romBank * 0x4000 + (addr & 0x3FFF)];
	}
	else if (addr >= 0xA000 && addr < 0xA200) {
		return ram[addr & 0xFF] & 0xF;
	}
	else {
		return 0;
	}
}


void writeByteMbc2(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val) {
	/* ROM */
	if (addr < 0x2000) {
		if (!(val & 0x10)) {
			switchEram = 1 - switchEram; // Enable external RAM
		}
	}
	else if (addr >= 0x2000 && addr < 0x4000) {
		if (val & 0x10) {
			romBank = val & 0xF;
		}
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xA200) {
		ram[addr & 0xFF] = val & 0xF;
	}
	else {
	}
}