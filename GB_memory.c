#include "GB_gameboy.h"
#include "GB_memory.h"
#include "GB_cpu.h"
#include "GB_video.h"
#include "GB_sound.h"
#include "GB_mbc.h"
#include <stdlib.h>
#include <string.h>

int memory_init(GB_MEMORY* mem) {
	mem->inBios = 0;

	mem->intEnMask = 0;
	mem->intFlag = 0;

	mem->joypadIn = 0;
	mem->joypadOut[0] = mem->joypadOut[1] = 0x0F;
	mem->__clocks = mem->tima = mem->tma = mem->tac = 0;

	mem->cartType = 0;
	return 0;
}

void memory_free(GB_MEMORY* mem) {
	free(mem->rom);
	free(mem->eram);
}

int readCartHeader(FILE * fp, GB_CARTHEADER *header) {
	GB_BYTE buffer[0x50] = { 0 }; // 100h-14Fh

	fseek(fp, 0x100, SEEK_SET);
	fread(buffer, 1, 0x50, fp);

	memset(header->title, 0, 17);
	memcpy(header->title, buffer + 0x34, 16);

	header->cartType = buffer[0x47];
	if (buffer[0x47] == 0) {
		header->mbcType = 0;
	}
	else if (buffer[0x47] >= 1 && buffer[0x47] <= 3) {
		header->mbcType = 1;
	}
	else if (buffer[0x47] >= 5 && buffer[0x47] <= 6) {
		header->mbcType = 2;
	}
	else if (buffer[0x47] >= 0xF && buffer[0x47] <= 0x13) {
		header->mbcType = 3;
	}
	else if (buffer[0x47] >= 0x19 && buffer[0x47] <= 0x1E) {
		header->mbcType = 5;
	}
	else { // Unknown
		printf("Unknown mbc type\n"); 
		return -1;
	}
	
	header->romSize = 0x8000 << (buffer[0x48] & 0xF); // 单位: BYTE
	header->romSize += ((buffer[0x48] >> 4) ? (0x8000 << (buffer[0x48] >> 4)) : 0);

	switch (buffer[0x49]) // RAM Size
	{
	case 0:	 header->ramSize = 0; break;
	case 1:	 header->ramSize = 0x800; break;
	case 2:	 header->ramSize = 0x2000; break;
	case 3:	 header->ramSize = 0x10000; break;
	case 4:	 header->ramSize = 0x40000; break;
	case 5:	 header->ramSize = 0x20000; break;
	default:  printf("Unknown ram size\n"); return -1;
	}

	return 0;
}

int loadCart(GB_MEMORY* mem, FILE* fp) {
	int res;
	res = readCartHeader(fp, &mem->cartHeader);
	if (res != 0) {
		printf("Read cartriage header failed\n");
		return -1;
	}

	switch (mem->cartHeader.mbcType)
	{
	case 0:
		mem->mbc.readByte = readByteDefault;
		mem->mbc.writeByte = writeByteDefault;
		break;
	case 1:
		mem->mbc.readByte = readByteMbc1;
		mem->mbc.writeByte = writeByteMbc1;
		break;
	case 2:
		mem->mbc.readByte = readByteMbc2;
		mem->mbc.writeByte = writeByteMbc2;
		break;
	case 3:
		mem->mbc.readByte = readByteMbc3;
		mem->mbc.writeByte = writeByteMbc3;
		break;
	case 5:
		mem->mbc.readByte = readByteMbc5;
		mem->mbc.writeByte = writeByteMbc5;
		break;
	}

	mem->rom = (GB_BYTE*)malloc(mem->cartHeader.romSize);
	mem->eram = (GB_BYTE*)malloc(0x2000);

	if (!mem->eram || !mem->rom) {
		free(mem->rom);
		free(mem->eram);
		return -1;
	}

	fseek(fp, 0, SEEK_SET);
	fread(mem->rom, 1, mem->cartHeader.romSize, fp);

	return 0;
}

int timer_step(GB_MEMORY* mem, int cycles) {
	if (mem->tac & 0b100) { // Timer Enabled
		int timer = mem->__clocks, last = mem->__clocks;
		int bitMask = ((mem->tac & 0b11) ? (0b10 << ((mem->tac & 0b11) << 1)) : 0b1000000000);

		for (int i = 4 * cycles; i > 0; i--) { // 逐个判断clocks
			timer++;
			if ((last & bitMask) && !(timer & bitMask)) {  // 检测下降沿个数
				mem->tima++;
				if (mem->tima == 0) {
					//i -= 4;
					mem->tima = mem->tma;
					mem->intFlag |= 0b00000100;
				}
			}
			last = timer;
		}
	}

	mem->__clocks += 4 * cycles;
	return 0;
}

GB_BYTE readByte(GB_MEMORY* mem, GB_WORD addr) {
	switch (addr & 0xF000)
	{
	case 0x0000: // BIOS (256b)/ROM0
		if (mem->inBios)
		{
			if (addr < 0x0100)
				return GB_bios[addr];
			else if (mem->parent->cpu->PC == 0x0101)
				mem->inBios = 0;
		}
	case 0x1000:
	case 0x2000:
	case 0x3000:
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		return mem->mbc.readByte(mem, addr);
	case 0x8000: // Graphics: VRAM (8k)
	case 0x9000:
		return mem->parent->gpu->vram[addr & 0x1FFF];
	case 0xA000: // External RAM (8k)
	case 0xB000:
		return mem->mbc.readByte(mem, addr);
	case 0xC000: // Working RAM (8k)
	case 0xD000:
	case 0xE000: // Working RAM shadow
		return mem->wram[addr & 0x1FFF];
	default: // case 0xF000: Working RAM shadow, I/O, Zero-page RAM
		if (addr < 0xFE00) return mem->wram[addr & 0x1FFF]; // 0xF000-0xFE00 Working RAM shadow
		else if (addr < 0xFEA0) return mem->parent->gpu->oam[addr & 0xA0]; // 0xFE00-0xFEA0 OAM


		else if (addr < 0xFF00) return mem->unimpledIoRegs[addr - 0xFEA0]; // 0xFEA0-0xFF00 unused I/O registers

		else if (addr == 0xFF00) return mem->joypadOut[mem->joypadIn]; // 0xFF00 joypad info
		//....many I/O registers....
		else if (addr == 0xFF04) return mem->__clocks >> 8; // 0xFF04 Divider
		else if (addr == 0xFF05) return mem->tima; // 0xFF05 Counter
		else if (addr == 0xFF06) return mem->tma;  // 0xFF06 Modulo
		else if (addr == 0xFF07) return mem->tac;  // 0xFF07 Counter Control
		else if (addr == 0xFF0F) return mem->intFlag; // 0xFF0F int flag
		else if (addr < 0xFF10) return mem->unimpledIoRegs[addr - 0xFEA0]; // 0xFF00-0xFF10 unimplemented I/O registers
		else if (addr < 0xFF40) return sound_readByte(mem->parent->sound, addr); // 0xFF10-0xFF40 sound

		else if (addr < 0xFF4C) return video_readByte(mem->parent->gpu, addr); // 0xFF40-0xFF4C GPU I/O registers

		else if (addr < 0xFF80) return mem->unimpledIoRegs[addr - 0xFEA0]; // 0xFF4C-0xFF80 unused I/O registers

		else if (addr < 0xFFFF) return mem->zram[addr & 0x7F]; // 0xFF80-0xFFFF ZERO RAM

		else return mem->intEnMask; // 0xFFFF
	}
}

void writeByte(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val) {
	switch (addr & 0xF000)
	{
	case 0x0000:
	case 0x1000:
	case 0x2000:
	case 0x3000:
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		mem->mbc.writeByte(mem, addr, val);
		return;
	case 0x8000: // Graphics: VRAM (8k)
	case 0x9000:
		mem->parent->gpu->vram[addr & 0x1FFF] = val;
		if (addr < 0x9800) video_updateTile(mem->parent->gpu, addr, val);
		return;
	case 0xA000: // External RAM (8k)
	case 0xB000:
		mem->eram[addr & 0x1FFF] = val;
		return;
	case 0xC000: // Working RAM (8k)
	case 0xD000:
	case 0xE000: // Working RAM shadow
		mem->wram[addr & 0x1FFF] = val;
		return;
	default: //case 0xF000: Working RAM shadow, I/O, Zero-page RAM
		if (addr < 0xFE00) mem->wram[addr & 0x1FFF] = val;  // 0xF000-0xFE00 Working RAM shadow
		else if (addr < 0xFEA0)  // 0xFE00-0xFEA0 OAM
		{
			mem->parent->gpu->oam[addr & 0xA0] = val;
			video_updateObj(mem->parent->gpu, addr, val);
		}

		else if (addr < 0xFF00) mem->unimpledIoRegs[addr - 0xFEA0] = val; // 0xFEA0-0xFF00 unused I/O registers

		else if (addr == 0xFF00) mem->joypadIn = (val & 0x20) >> 5; // 0xFF00 joypad info (Power On)
		//....many I/O registers....
		else if (addr == 0xFF04) { // 0xFF04 Divider
			int bitMask = ((mem->tac & 0b11) ? (0b10 << ((mem->tac & 0b11) << 1)) : 0b1000000000);
			if (mem->__clocks & bitMask) {  // 原来是1, 现在变0, 产生一个下降沿
				mem->tima++;
				if (mem->tima == 0) { // 溢出的下一个周期才会load, 此处忽略
					mem->tima = mem->tma;
					mem->intFlag |= 0b00000100;
				}
			}
			mem->__clocks = 0;
		}
		else if (addr == 0xFF05) mem->tima = val; // 0xFF05 Counter
		else if (addr == 0xFF06) mem->tma = val;  // 0xFF06 Modulo
		else if (addr == 0xFF07) mem->tac = val & 0b111;  // 0xFF07 Counter Control
		else if (addr == 0xFF0F) mem->intFlag = val; // 0xFF0F int flag
		//....many I/O registers....
		else if (addr < 0xFF10) mem->unimpledIoRegs[addr - 0xFEA0] = val; // 0xFEA0-0xFF10 unimplemented I/O registers
		else if (addr < 0xFF40) sound_writeByte(mem->parent->sound, addr, val); // 0xFF10-0xFF40 sound

		else if (addr < 0xFF4C) video_writeByte(mem->parent->gpu, addr, val); // 0xFF40-0xFF4C GPU I/O registers

		else if (addr < 0xFF80) mem->unimpledIoRegs[addr - 0xFEA0] = val; // 0xFF4C-0xFF80 unused I/O registers

		else if (addr < 0xFFFF) mem->zram[addr & 0x7F] = val; // 0xFF80-0xFFFF ZERO RAM

		else mem->intEnMask = val; // 0xFFFF
	}
}

GB_WORD readWord(GB_MEMORY* mem, GB_WORD addr) {
	return readByte(mem, addr) + (readByte(mem, addr + 1) << 8);
}

void writeWord(GB_MEMORY* mem, GB_WORD addr, GB_WORD val) {
	writeByte(mem, addr, val & 0xFF);
	writeByte(mem, addr + 1, val >> 8);
}
