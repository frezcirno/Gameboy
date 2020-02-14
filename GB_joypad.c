#include <SDL.h>
#include "GB_memory.h"
#include "GB_gameboy.h"

int ch1 = 1, ch2 = 1, ch3 = 1, ch4 = 1;

int check_joypad(GB_MEMORY* mem, int cycles) { // Event Handle
	static int counter = 0;
	counter -= cycles;
	if (counter <= 0) { // 1048576*t/1000 T = t ms
		counter += 10;// 1048576 * 1 / 80 / 1000;
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type)
			{
			case SDL_QUIT:	mem->parent->run = 0;	break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				case SDLK_DOWN:		mem->joypadOut[1] &= 0b0111; break;
				case SDLK_UP:		mem->joypadOut[1] &= 0b1011; break;
				case SDLK_LEFT:		mem->joypadOut[1] &= 0b1101; break;
				case SDLK_RIGHT:	mem->joypadOut[1] &= 0b1110; break;
				case SDLK_RETURN:	mem->joypadOut[0] &= 0b0111; break;
				case SDLK_SPACE:	mem->joypadOut[0] &= 0b1011; break;
				case SDLK_x:		mem->joypadOut[0] &= 0b1101; break;
				case SDLK_z:		mem->joypadOut[0] &= 0b1110; break;
				case SDLK_F5:       ch1 = 1 - ch1; break;
				case SDLK_F6:       ch2 = 1 - ch2; break;
				case SDLK_F7:       ch3 = 1 - ch3; break;
				case SDLK_F8:       ch4 = 1 - ch4; break;
				default: break;
				}
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.sym)
				{
				case SDLK_DOWN:		mem->joypadOut[1] |= 0b1000; break;
				case SDLK_UP:		mem->joypadOut[1] |= 0b0100; break;
				case SDLK_LEFT:		mem->joypadOut[1] |= 0b0010; break;
				case SDLK_RIGHT:	mem->joypadOut[1] |= 0b0001; break;
				case SDLK_RETURN:	mem->joypadOut[0] |= 0b1000; break;
				case SDLK_SPACE:	mem->joypadOut[0] |= 0b0100; break;
				case SDLK_x:		mem->joypadOut[0] |= 0b0010; break;
				case SDLK_z:		mem->joypadOut[0] |= 0b0001; break;
				default: break;
				}
				break;
			default:	break;
			}
		}
	} // Loop
	return 0;
}
