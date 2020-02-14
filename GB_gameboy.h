#pragma once

struct GB_MEMORY;
struct GB_CPU;
struct GB_VIDEO;
struct GB_SOUND;

typedef struct GB_GAMEBOY {
	int run;
	struct GB_MEMORY* mem;
	struct GB_CPU* cpu;
	struct GB_VIDEO* gpu;
	struct GB_SOUND* sound;
}GB_GAMEBOY;

int gameboy_init(GB_GAMEBOY* gb);

int gameboy_free(GB_GAMEBOY* gb);
