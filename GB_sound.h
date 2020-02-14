#pragma once
#include <SDL.h>

typedef struct GB_SOUND {
	struct GB_GAMEBOY* parent;
	SDL_AudioSpec spec; // freq采样率: 每秒钟多少次采样
	Uint8* playBuf;

	struct { // Sound 1 变频方波
		GB_BYTE swpPeriod, swpDir, swpNum, __swpCnt;  // 0xFF10 NR10 -PPP NSSS Sweep period, negate, shift
		GB_BYTE duty, lenCnt, _enable, __dutyStep;                 // 0xFF11 NR11 DDLL LLLL Duty, Length load (64-L)/256
		GB_BYTE envInit, envDir, envPeriod, __envCnt, __vol;       // 0xFF12 NR12 VVVV APPP Starting volume, Envelope add mode, period
		GB_WORD freq, __freq;		                               // 0xFF13 NR13 FFFF FFFF Frequency
		GB_BYTE lenCntEn;					                       // 0xFF14 NR14 TL-- -FFF Trigger, Length enable, Frequency MSB
		
		int __freqTimer; // (2048 - sound->sqr1.freq) 个周期一个信号
	} sqr1;

	struct { // Sound 2 定频方波
		GB_BYTE duty, lenCnt, _enable, __dutyStep;                 // 0xFF16 NR21 DDLL LLLL Duty, Length load(64 - L)
		GB_BYTE envInit, envDir, envPeriod, __envCnt, __vol;       // 0xFF17 NR22 VVVV APPP Starting volume, Envelope add mode, period
		GB_WORD freq;                                              // 0xFF18 NR23 FFFF FFFF Frequency LSB
		GB_BYTE lenCntEn;					                   	   // 0xFF19 NR24 TL-- - FFF Trigger, Length enable, Frequency MSB
		
		int __freqTimer; // (2048 - sound->sqr1.freq) 个周期一个信号
	} sqr2;

	struct { // Sound 3 波表
		GB_BYTE output;	          // 0xFF1A NR30 E--- ----DAC power
		GB_WORD lenCnt;           // 0xFF1B NR31 LLLL LLLL Length load(256 - L)
		GB_BYTE _enable;
		GB_BYTE volLevel;         // 0xFF1C NR32 -VV- ----Volume code(00 = 0 %, 01 = 100 %, 10 = 50 %, 11 = 25 %)
		GB_WORD freq;	          // 0xFF1D NR33 FFFF FFFF Frequency LSB
		GB_BYTE lenCntEn;         // 0xFF1E NR34 TL-- -FFF Trigger, Length enable, Frequency MSB
		
		int __freqTimer;
		
		GB_BYTE pattern[0x20]; // 0xFF30-0xFF40 波表
		GB_BYTE __samplePos;
	} wave;

	struct { // Sound 4 噪声
		GB_BYTE lenCnt, _enable;                              // 0xFF20 NR41 --LL LLLL Length load(64 - L)
		GB_BYTE envInit, envDir, envPeriod, __envCnt, __vol;  // 0xFF21 NR42 VVVV APPP Starting volume, Envelope add mode, period
		GB_BYTE freqShift, freqWidth, freqRatio;              // 0xFF22 NR43 SSSS WDDD Clock shift, Width mode of LFSR, Divisor code
		GB_WORD freq; // Frequency = 524288 Hz / r / 2^(s+1) ;For r=0 assume r=0.5 instead
		GB_BYTE lenCntEn;                                     // 0xFF23 NR44 TL-- ---- Trigger, Length enable

		int __freqTimer;
	} noise;

	struct { // Control
		GB_BYTE volLeft, volRight; // 0xFF24 NR50 bit0-2, bit4-6
		GB_BYTE vinLeft, vinRight; // 0xFF24 NR50 bit3, bit7
		GB_BYTE ch1Left, ch1Right; // 0xFF25 NR51
		GB_BYTE ch2Left, ch2Right; // 0xFF25 NR51
		GB_BYTE ch3Left, ch3Right; // 0xFF25 NR51
		GB_BYTE ch4Left, ch4Right; // 0xFF25 NR51
		GB_BYTE switchAll;	// 0xFF26 NR52  bit7 总开关, bit 0-3 lenFlag
	} ctrl;
} GB_SOUND;

int sound_init(GB_SOUND* sound);

void sound_free(GB_SOUND* sound);

int sound_step(GB_SOUND* sound, int cycles);

int genSample1(GB_SOUND* sound); 

void enQueue(GB_SOUND* sound, Uint8 leftSig, Uint8 rightSig);

GB_BYTE sound_readByte(GB_SOUND* sound, GB_WORD addr);

void sound_writeByte(GB_SOUND* sound, GB_WORD addr, GB_BYTE val);
