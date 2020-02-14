#include "GB_memory.h"

static GB_BYTE switchEram = 1;  // 0x0000-0x2000 ÍØÕ¹ram¿ª¹Ø(W)
static GB_WORD romBank = 1;   // 0x2000-0x4000 Switch between 1-31 (value 0 is seen as 1)
static GB_BYTE ramBank = 0;  // 0x4000-0x6000 Switch between 0-F, if any

GB_BYTE readByteMbc5(GB_MEMORY* mem, GB_WORD addr) {
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


void writeByteMbc5(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val) {
	/* ROM */
	if (addr < 0x2000) {
		switchEram = (((val & 0x0F) == 0x0A) ? 1 : 0); // Enable external RAM
	}
	else if (addr < 0x3000) {
		romBank = (romBank & 0xFF00) | val;
	}
	else if (addr < 0x4000) {
		romBank = (val << 8) | (romBank & 0xFF);
	}
	else if (addr < 0x6000) {
		ramBank = val;
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		mem->eram[ramBank * 0x2000 + addr & 0x1FFF] = val;
	}
	else {
	}
}