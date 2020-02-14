#include "GB_memory.h"
#include <time.h>

static GB_BYTE romBank = 1;   // 0x2000-0x4000 Switch between 1-127
static GB_BYTE switchEram = 1;  // 0x0000-0x2000 Eram/RTC¿ª¹Ø(W)
static GB_BYTE mode = 0; // 0x4000-0x6000 Switch between 0-3
static GB_BYTE rtc;
/*
mode=0-3 RAM Mode
mode=8-C RTC Mode
*/

GB_BYTE readByteMbc3(GB_MEMORY* mem, GB_WORD addr) {
	/* ROM */
	if (addr < 0x4000) {
		return mem->rom[addr];
	}
	else if (addr < 0x8000) {
		return mem->rom[romBank * 0x4000 + (addr & 0x3FFF)];
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		if (mode <= 3)
			return mem->eram[mode * 0x2000 + (addr & 0x1FFF)];
		else {
			return rtc;
		}
	}
	else {
		return 0;
	}
}


void writeByteMbc3(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val) {
	static int preLatch = 0;
	/* ROM */
	if (addr < 0x2000) {
		switchEram = (((val & 0x0F) == 0x0A) ? 1 : 0); // Enable external RAM
	}
	else if (addr < 0x4000) {
		if (!val) val = 1;
		romBank = val & 0x7F;
	}
	else if (addr < 0x6000) {
		mode = val;
	}
	else if (addr < 0x8000) {
		if (val == 0) preLatch = 1;
		else if (val == 1 && preLatch) {
			preLatch = 0;
			switch (mode) {
				case 0x8:rtc = 0xcd; break;
				case 0x9:rtc = 0xcd; break;
				case 0xA:rtc = 0xcd; break;
				case 0xB:rtc = 0xcd; break;
				case 0xC:rtc = 0xcd; break;
			}
		}
	}
	/* RAM */
	else if (addr >= 0xA000 && addr < 0xC000) {
		mem->eram[mode * 0x2000 + addr & 0x1FFF] = val;
	}
	else {
	}
}