#include "GB_memory.h"

GB_BYTE readByteDefault(GB_MEMORY* mem, GB_WORD addr) {
	/* ROM */
	if (addr < 0x8000) {
		return mem->rom[addr];
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		return mem->eram[addr & 0x1FFF];
	}
	else {
		return 0;
	}
}


void writeByteDefault(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val) {
	/* ROM */
	if (addr < 0x8000) {
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		mem->eram[addr & 0x1FFF] = val;
	}
	else {
	}
}