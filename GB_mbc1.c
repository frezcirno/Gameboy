#include "GB_memory.h"

static GB_BYTE mode = 0;
 /*
	 0: ROM 模式 128 ROM-Bank(16KB) + 1 ERAM-Bank(8KB)
	 1: RAM 模式 32 ROM-Bank + 4 ERAM-Bank 
  */
static GB_BYTE switchEram = 1;  // 0x0000-0x2000 拓展ram开关(W)
static GB_BYTE romBank = 1;   // 0x2000-0x4000 Switch between 1-31 (value 0 is seen as 1)
static GB_BYTE ramBank = 0;  // 0x4000-0x6000 Switch between 0-3

GB_BYTE readByteMbc1(GB_MEMORY* mem, GB_WORD addr) {
	/* ROM */
	if (addr < 0x4000) {
		return mem->rom[addr];
	}
	else if (addr < 0x8000) {
		return mem->rom[romBank * 0x4000 + (addr & 0x3FFF)];
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		return mem->eram[ramBank * 0x2000 + (addr & 0x1FFF)];
	}
	else {
		return 0;
	}
}


void writeByteMbc1(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val) {
	/* ROM */
	if (addr < 0x2000) {
		switchEram = (((val & 0x0F) == 0x0A) ? 1 : 0); // Enable external RAM
	}
	else if (addr < 0x4000) {
		val &= 0b11111;
		if (!val) val = 1; // skipping #0
		romBank = (romBank & 0b1100000) | val;
	}
	else if (addr < 0x6000) {
		if (mode == 0) {// RAM mode: Set bank
			ramBank = val & 3;
		}
		else {// ROM mode: Set high bits of bank
			romBank = (romBank & 0x1F) + ((val & 3) << 5);
		}
	}
	else if (addr < 0x8000) {
		mode = val & 1;
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		mem->eram[ramBank * 0x2000 + addr & 0x1FFF] = val;
	}
	else {
	}
}