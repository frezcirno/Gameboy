
CC = gcc
CFLAGS = -Wall -O2 -I. `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs`

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

gameboy: GB_main.o GB_gameboy.o GB_cpu.o GB_memory.o GB_joypad.o \
GB_mbc0.o GB_mbc1.o GB_mbc2.o GB_mbc3.o GB_mbc5.o \
GB_video.o GB_sound.o 
	$(CC) $^ $(LDFLAGS) -o $@ 

clean: 
	-rm ./*.aps
	-rm ./*.gch
	-rm ./*.o
	-rm gameboy