#define _CRT_SECURE_NO_WARNINGS
#include "GB_gameboy.h"
#include "GB_video.h"
#include "GB_cpu.h"
#include "GB_sound.h"
#include "GB_joypad.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
	char fileName[500];
	FILE* fp;
	int res;

	printf("Please input file path: ");
	if (fgets(fileName, sizeof(fileName), stdin) == NULL) {
		printf("Read error\n");
		return -1;
	}

	for (int i = strlen(fileName) - 1; i >= 0; i--) {
		if (fileName[i] == '\n') fileName[i] = 0;
	}
	
	fp = fopen(fileName, "r");
	if (!fp) {
		printf("Open file failed\n");
		return -1;
	}

	GB_GAMEBOY* gb = (GB_GAMEBOY*)malloc(sizeof(GB_GAMEBOY));
	if (!gb) {
		printf("Malloc failed\n");
		return -1;
	}

	res = gameboy_init(gb);
	if (res != 0) {
		printf("Gameboy init failed\n");
		return -1;
	}

	res = loadCart(gb->mem, fp);
	if (res != 0) {
		printf("Load cartridge failed\n");
		return -1;
	}

	fclose(fp);

	SDL_SetWindowTitle(gb->gpu->window, gb->mem->cartHeader.title);

	while (gb->run)
	{
//#define GBDB 
#ifdef GBDB
		static GB_WORD cnt = 0, cntEn = 0, stop = 0x101;
		printf(" %04x %04x %04x %04x %04x %04x\n", gb->cpu->PC, gb->cpu->SP, gb->cpu->AF, gb->cpu->BC, gb->cpu->DE, gb->cpu->HL);
		if (gb->cpu->PC == stop || (cntEn && cnt == 0)) {
			unsigned op = 0;
			if (cnt == 0) cntEn = 0;
			printf(">>>");
			scanf("%x", &op);
			if ((op & 0xF000) == 0xD000) { // daojishi
				cnt = op & 0xFFF;
				cntEn = 1;
			}
			else {
				stop = op;
			}
		}
		if (cntEn) cnt--;
#endif
		int usedCycles = cpu_step(gb->cpu);

		timer_step(gb->mem, usedCycles);

		video_step(gb->gpu, usedCycles);

		check_joypad(gb->mem, usedCycles);

		sound_step(gb->sound,usedCycles);

		const char* err = SDL_GetError();
		if (*err) {
			SDL_Log("%s", err);
		}
	}

	gameboy_free(gb);
	free(gb);
	return 0;
}
