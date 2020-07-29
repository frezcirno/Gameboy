#include "GB_common.h"
#include "GB_sound.h"
#include "GB_gameboy.h"

#include <string.h>
#include <stdlib.h>

extern int ch1, ch2, ch3, ch4;

int sound_init(GB_SOUND* sound)
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    const int count = SDL_GetNumAudioDevices(0);
    for (int index = 0; index < count; ++index) {
        SDL_Log("Audio device %d: %s\n", index, SDL_GetAudioDeviceName(index, 0));
    }

    memset(&sound->spec, 0, sizeof(sound->spec));
    sound->spec.freq = 48000; // 一秒钟 48000 次采样
    sound->spec.format = AUDIO_U8; // 每个采样数据占 1 个字节
    sound->spec.channels = 2; // 左右声道 (每次采样两个数据)
    sound->spec.samples = 1000; // 音频缓冲区大小 (单位: 采样次数) // 影响音频填充完毕后的等待时间

    if (SDL_OpenAudio(&sound->spec, NULL) < 0) { //打开id==1的音频设备
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        return -1;
    }

    sound->playBuf = (Uint8*)malloc(sound->spec.size);
	if (!sound->playBuf) {
        return -1;
	}

    memset(sound->playBuf, 0, sound->spec.size);

    SDL_PauseAudio(0);
    return 0;
}

void sound_free(GB_SOUND* sound)
{
    free(sound->playBuf);
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int sound_step(GB_SOUND* sound, int cycles)   // 平均每 1-6 T 调用一次, 1T = 1 / 1, 048, 576s
{
    static const short clockpattern256[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    static const short clockpattern128[8] = {0, 0, 1, 0, 0, 0, 1, 0};
    static const short clockpattern64[8]  = {0, 0, 0, 0, 0, 0, 0, 1};
	static int frameTimer = 0, frameTimerPhase = 0; // 512Hz Common Timer
	static long long cntPushSound = 0, cntPushSoundPeriod = 375 * 1048576 / 48000, cntPushSoundPeriodFix = 0;
	int clk256Hz = 0, clk128Hz = 0, clk64Hz = 0;

	/* 512Hz 时钟 */
    frameTimer -= cycles;
    if (frameTimer <= 0) {
        frameTimer += GB_CPU_FREQ / 512;
        frameTimerPhase = (frameTimerPhase + 1) & 7;
        clk256Hz = clockpattern256[frameTimerPhase];
        clk128Hz = clockpattern128[frameTimerPhase];
        clk64Hz = clockpattern64[frameTimerPhase];
    }

	/* Channel 1 时钟 */
	sound->sqr1.__freqTimer -= cycles;
	if (sound->sqr1.__freqTimer <= 0) { // duty 1
		sound->sqr1.__freqTimer += (2048 - sound->sqr1.freq) / 4;
		sound->sqr1.__dutyStep = (sound->sqr1.__dutyStep + 1) & 7;
	}

	/* Channel 2 时钟 */
	sound->sqr2.__freqTimer -= cycles;
	if (sound->sqr2.__freqTimer <= 0) { // duty 2
		sound->sqr2.__freqTimer += (2048 - sound->sqr2.freq) / 4;
		sound->sqr2.__dutyStep = (sound->sqr2.__dutyStep + 1) & 7;
	}

	/* Channel 3 时钟 */
	sound->wave.__freqTimer -= cycles;
	if (sound->wave.__freqTimer < 0) { // sample pos 3
		sound->wave.__freqTimer += (2048 - sound->wave.freq) / 2;
		sound->wave.__samplePos = (sound->wave.__samplePos + 1) & 31;
	}

	/* Channel 4 时钟 */
	sound->noise.__freqTimer -= cycles;
	if (sound->noise.__freqTimer < 0) { // sample pos 3
		sound->noise.__freqTimer += (2048 - sound->noise.freq) / 4;
	}

    if (clk128Hz) { // Update Freq Sweep (128 Hz)
        if (sound->sqr1._enable && sound->sqr1.swpPeriod && sound->sqr1.swpNum) { // swpNum == 0 时 频率也不会更新
            sound->sqr1.__swpCnt--;
            if (sound->sqr1.__swpCnt == 0) {
                sound->sqr1.__swpCnt = sound->sqr1.swpPeriod;
                // Sweep 频率更新 (1-7)/128 Hz
                /*if (nextFreq > 2047) { sound->sqr1.__swpFlag = 0; } else */
                // restart处的检测和二次检测已经保证此处一定不会溢出
                // OK AND 变化系数不为零, 写入
                GB_WORD nextFreq = (sound->sqr1.swpDir ?
                                    sound->sqr1.__freq - (sound->sqr1.__freq >> sound->sqr1.swpNum) :
                                    sound->sqr1.__freq + (sound->sqr1.__freq >> sound->sqr1.swpNum));
                if (nextFreq < 0) nextFreq = 0;
                if (nextFreq < 2048){
                    sound->sqr1.freq = nextFreq;
                    sound->sqr1.__freq = nextFreq;
                } else {
                    sound->sqr1._enable = 0;
                }
                sound->sqr1.swpNum--; // 减到零就停止
            }
        }
    }

    if (clk256Hz) { // Update Length Counter (256 Hz)
        if (sound->sqr1.lenCntEn && sound->sqr1.lenCnt) {
            sound->sqr1.lenCnt--;
            if (sound->sqr1.lenCnt == 0) {
                sound->sqr1._enable = 0;
            }
        }
        if (sound->sqr2.lenCntEn && sound->sqr2.lenCnt) {
            sound->sqr2.lenCnt--;
            if (sound->sqr2.lenCnt == 0) {
                sound->sqr2._enable = 0;
            }
        }
        if (sound->wave.lenCntEn && sound->wave.lenCnt) {
            sound->wave.lenCnt--;
            if (sound->wave.lenCnt == 0) {
                sound->wave._enable = 0;
            }
        }
        if (sound->noise.lenCntEn && sound->noise.lenCnt) {
            sound->noise.lenCnt--;
            if (sound->noise.lenCnt == 0) {
                sound->noise._enable = 0;
            }
        }
    }

    if (clk64Hz) { // Update Volume Envelope (64 Hz)
		if (sound->sqr1.envPeriod) { // Enable volume envelope
			sound->sqr1.__envCnt--; 
			if (!sound->sqr1.__envCnt) { // Count for n times
				sound->sqr1.__envCnt = sound->sqr1.envPeriod; // execute & reload
				if (sound->sqr1.envDir) {
					if (sound->sqr1.__vol < 15) sound->sqr1.__vol++;
				}
				else {
					if (sound->sqr1.__vol > 0) sound->sqr1.__vol--;
				}
			}
		}
		if (sound->sqr2.envPeriod) {
			sound->sqr2.__envCnt--;
			if (!sound->sqr2.__envCnt) {
				sound->sqr2.__envCnt = sound->sqr2.envPeriod;
				if (sound->sqr2.envDir) {
					if (sound->sqr2.__vol < 15) sound->sqr2.__vol++;
				}
				else {
					if (sound->sqr2.__vol > 0) sound->sqr2.__vol--;
				}
			}
		}
		if (sound->noise.envPeriod) {
			sound->noise.__envCnt--; // envelope 4
			if (!sound->noise.__envCnt) {
				sound->noise.__envCnt = sound->noise.envPeriod;
				if (sound->noise.envDir) {
					if (sound->noise.__vol < 15) sound->noise.__vol++;
				}
				else {
					if (sound->noise.__vol > 0) sound->noise.__vol--;
				}
			}
		}
    }

	cntPushSound -= cycles * 375LL;
	if (cntPushSound <= 0) {
		genSample1(sound);
		cntPushSound += cntPushSoundPeriod;
	}
    return 0;
}

void enQueue(GB_SOUND* sound, Uint8 leftSig, Uint8 rightSig) {
	static int bytePos = 0;
	sound->playBuf[bytePos++] = leftSig;
	sound->playBuf[bytePos++] = rightSig;
	if (bytePos == sound->spec.size) {
		SDL_QueueAudio(1, sound->playBuf, sound->spec.size);
		memset(sound->playBuf, 0, sound->spec.size);
		bytePos = 0;
	}
}

int genSample1(GB_SOUND* sound) {
	static const sqrWave[4][8] = {
		{0, 0, 0, 0, 0, 0, 1, 2},
		{1, 1, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 1, 1, 2, 2},
		{1, 1, 2, 2, 2, 2, 1, 1}
	};
	Uint8 sig1 = 0, sig2 = 0, sig3 = 0, sig4 = 0;
	Uint8 leftSig = 0, rightSig = 0;

	if (ch1 && sound->sqr1._enable && sound->sqr1.__vol){// && sound->sqr1.freq < (2048 - 131072 / 2048)) {
		sig1 = sqrWave[sound->sqr1.duty][sound->sqr1.__dutyStep] * sound->sqr1.__vol;
	}
	if (ch2 && sound->sqr2._enable && sound->sqr2.__vol){// && sound->sqr2.freq < (2048 - 131072 / 2048)) {
		sig2 = sqrWave[sound->sqr2.duty][sound->sqr2.__dutyStep] * sound->sqr2.__vol;
	}
	if (ch3 && sound->wave.output && sound->wave._enable && sound->wave.volLevel && sound->wave.freq <= 2041) {
		GB_BYTE vol = 0b100 >> (sound->wave.volLevel - 1); // wave.volLevel: 0静音, 1-3右移x-1位
		sig3 = vol * sound->wave.pattern[sound->wave.__samplePos];
	}
	if (ch4 && sound->noise._enable && sound->noise.__vol) {
		sig4 = sound->noise.__vol * (rand() % 2);
	}

	/*
	SDL_MixAudio(&leftSig, &sig1, 1, sound->ctrl.ch1Left * SDL_MIX_MAXVOLUME);
	SDL_MixAudio(&leftSig, &sig2, 1, sound->ctrl.ch2Left * SDL_MIX_MAXVOLUME);
	SDL_MixAudio(&leftSig, &sig3, 1, sound->ctrl.ch3Left * SDL_MIX_MAXVOLUME);
	SDL_MixAudio(&leftSig, &sig4, 1, sound->ctrl.ch4Left * SDL_MIX_MAXVOLUME);

	SDL_MixAudio(&rightSig, &sig1, 1, sound->ctrl.ch1Right * SDL_MIX_MAXVOLUME);
	SDL_MixAudio(&rightSig, &sig2, 1, sound->ctrl.ch2Right * SDL_MIX_MAXVOLUME);
	SDL_MixAudio(&rightSig, &sig3, 1, sound->ctrl.ch3Right * SDL_MIX_MAXVOLUME);
	SDL_MixAudio(&rightSig, &sig4, 1, sound->ctrl.ch4Right * SDL_MIX_MAXVOLUME);
	*/
	
	leftSig += sound->ctrl.ch1Left * sig1;
	leftSig += sound->ctrl.ch2Left * sig2;
	leftSig += sound->ctrl.ch3Left * sig3;
	leftSig += sound->ctrl.ch4Left * sig4;
	leftSig *= sound->ctrl.volLeft;
	leftSig *= sound->ctrl.switchAll;

	rightSig += sound->ctrl.ch1Right * sig1;
	rightSig += sound->ctrl.ch2Right * sig2;
	rightSig += sound->ctrl.ch3Right * sig3;
	rightSig += sound->ctrl.ch4Right * sig4;
	rightSig *= sound->ctrl.volRight;
	rightSig *= sound->ctrl.switchAll;
	
	enQueue(sound, leftSig, rightSig);

	return 0;
}

GB_BYTE sound_readByte(GB_SOUND* sound, GB_WORD addr)
{
    if (addr >= 0xFF30) {
        return (sound->wave.pattern[(addr - 0xFF30) * 2] << 4) |
               (sound->wave.pattern[(addr - 0xFF30) * 2 + 1] & 0xF);
    }
    switch (addr) {
    /* Channel 1 */
    case 0xFF10:
        return (sound->sqr1.swpPeriod << 4) |
               (sound->sqr1.swpDir ? 0x8 : 0) |
               (sound->sqr1.swpNum);
    case 0xFF11:
        return sound->sqr1.duty << 6;
    case 0xFF12:
        return (sound->sqr1.envInit << 4) |
               (sound->sqr1.envDir ? 0x8 : 0x0) |
               (sound->sqr1.envPeriod & 0x7);
    case 0xFF13:
		return 0;
    case 0xFF14:
        return sound->sqr1.lenCntEn << 6; //  Bit 6 - Counter/consecutive selection

    /* Channel 2 */
    case 0xFF15:
		return 0;
    case 0xFF16:
        return sound->sqr2.duty << 6;
    case 0xFF17:
        return (sound->sqr2.envInit << 4) |
               (sound->sqr2.envDir ? 0x8 : 0x0) |
               (sound->sqr2.envPeriod & 0x7);
    case 0xFF18:
		return 0;
    case 0xFF19:
        return sound->sqr2.lenCntEn << 6; //  Bit 6 - Counter/consecutive selection

    /* Channel 3 */
    case 0xFF1A:
        return sound->wave.output << 7;
    case 0xFF1B:
        return (256 - sound->wave.lenCnt); // Not sure
    case 0xFF1C:
        return (sound->wave.volLevel & 0x3) << 5;
    case 0xFF1D:
		return 0;
    case 0xFF1E:
        return sound->wave.lenCntEn << 6;

    /* Channel 4 */
    case 0xFF1F:
		return 0;
    case 0xFF20:
        return (64 - sound->noise.lenCnt); // Not sure
    case 0xFF21:
        return (sound->noise.envInit << 4) |
               (sound->noise.envDir ? 0x8 : 0x0) |
               (sound->noise.envPeriod);
    case 0xFF22:
        return (sound->noise.freqShift << 4) |
               (sound->noise.freqWidth << 3) |
               (sound->noise.freqRatio);
    case 0xFF23:
        return sound->wave.lenCntEn << 6;

    /* Control */
    case 0xFF24:
        return (sound->ctrl.vinRight << 7) |
               ((sound->ctrl.volRight & 0x3) << 4) |
               (sound->ctrl.vinLeft << 3) |
               (sound->ctrl.volLeft & 0x3);
    case 0xFF25:
        return (sound->ctrl.ch4Right << 7) |
               (sound->ctrl.ch3Right << 6) |
               (sound->ctrl.ch2Right << 5) |
               (sound->ctrl.ch1Right << 4) |
               (sound->ctrl.ch4Left << 3) |
               (sound->ctrl.ch3Left << 2) |
               (sound->ctrl.ch2Left << 1) |
               (sound->ctrl.ch1Left);
    case 0xFF26:
        return (sound->ctrl.switchAll << 7) |
               (sound->noise._enable << 3) |
               (sound->wave._enable << 2) |
               (sound->sqr2._enable << 1) |
               (sound->sqr1._enable);
    default:
        return 0;
    }
}

void sound_writeByte(GB_SOUND* sound, GB_WORD addr, GB_BYTE val)
{
    if (addr >= 0xFF30) {
        sound->wave.pattern[(addr - 0xFF30) * 2] = val >> 4;
        sound->wave.pattern[(addr - 0xFF30) * 2 + 1] = val & 0xF;
        return;
    }
    switch (addr) {
    /* Channel 1 */
    case 0xFF10:  // -PPP NSSS
        sound->sqr1.swpPeriod = (val >> 4) & 0x7;
        sound->sqr1.swpDir = (val & 0x8 ? 1 : 0);
        sound->sqr1.swpNum = val & 0x7;
        break;
    case 0xFF11:  // DDLL LLLL
        sound->sqr1.duty = val >> 6;
        sound->sqr1.lenCnt = 64 - (val & 0x3F);
        break;
    case 0xFF12:  // VVVV APPP
        sound->sqr1.envInit = val >> 4;
        sound->sqr1.envDir = (val & 0x8 ? 1 : 0);
        sound->sqr1.envPeriod = val & 0x7;
        break;
    case 0xFF13:  // FFFF FFFF
        sound->sqr1.freq = (sound->sqr1.freq & 0xFF00) | val;
        break;
    case 0xFF14:  // TL-- -FFF
        sound->sqr1.lenCntEn = (val & 0x40 ? 1 : 0);
        sound->sqr1.freq = ((val & 0x3) << 8) | (sound->sqr1.freq & 0xFF);
        if (val & 0x80){
			sound->sqr1.__freqTimer = (2048 - sound->sqr1.freq) / 4;
            //sweep 初始化
            sound->sqr1.__freq = sound->sqr1.freq;
            sound->sqr1.__swpCnt = sound->sqr1.swpPeriod;
            /*
            if (sound->sqr1.__swpFlag && sound->sqr1.swpNum) { // 变化系数非零, 立即执行一次频率更新溢出检测 (若ch1已禁用,检测无意义)
                GB_WORD newFreq = (sound->sqr1.swpDir ?
                                   sound->sqr1.__freq - (sound->sqr1.__freq >> sound->sqr1.swpNum) :
                                   sound->sqr1.__freq + (sound->sqr1.__freq >> sound->sqr1.swpNum));
                if (newFreq > 2047) { // If overflow, disable channel 1
                    sound->sqr1.__swpFlag = 0;
                }
            }*/

            // Length Counter 初始化
            if (sound->sqr1.lenCnt == 0) sound->sqr1.lenCnt = 64; // reset
            // Envelope 初始化
            sound->sqr1.__envCnt = sound->sqr1.envPeriod;
            sound->sqr1.__vol = sound->sqr1.envInit;
            // DAC power
            if (!sound->sqr1.envInit && !sound->sqr1.envDir) { sound->sqr1._enable = 0; }
            else { sound->sqr1._enable = 1; } // reset, on
        }
        break;

    /* Channel 2 */
    case 0xFF16:
        sound->sqr2.duty = val >> 6;
        sound->sqr2.lenCnt = 64 - (val & 0x3F);
        break;
    case 0xFF17:
        sound->sqr2.envInit = val >> 4;
        sound->sqr2.envDir = (val & 0x8 ? 1 : 0);
        sound->sqr2.envPeriod = val & 0x7;
        break;
    case 0xFF18:
        sound->sqr2.freq = (sound->sqr2.freq & 0xFF00) | val;
        break;
    case 0xFF19:
        sound->sqr2.lenCntEn = (val & 0x40 ? 1 : 0);
        sound->sqr2.freq = ((val & 0x3) << 8) | (sound->sqr2.freq & 0xFF);
        if (val & 0x80){
            // Length Counter 初始化
            if (sound->sqr2.lenCnt == 0) sound->sqr2.lenCnt = 64;

            // Envelope 初始化
            sound->sqr2.__envCnt = sound->sqr2.envPeriod;
            sound->sqr2.__vol = sound->sqr2.envInit;

            // DAC power
            if (!sound->sqr2.envInit && !sound->sqr2.envDir) { sound->sqr2._enable = 0; }
            else { sound->sqr2._enable = 1; } // reset, on
        }
        break;

    /* Channel 3 */
    case 0xFF1A:
		sound->wave.output = (val >> 7);
		if (sound->wave.output == 0) sound->wave._enable = 0;
		// else sound->wave._enable = 1;
        break;
    case 0xFF1B:
        sound->wave.lenCnt = 256 - val;
        break;
    case 0xFF1C:
        sound->wave.volLevel = (val >> 5) & 0x3;
        break;
    case 0xFF1E:
        sound->wave.freq = (sound->wave.freq & 0xF0) | val;
        break;
    case 0xFF1F:
        sound->wave.lenCntEn = (val & 0x40 ? 1 : 0);
		sound->wave.freq = (((val << 8) & 0x30) | (sound->wave.freq & 0xF));
        if (val & 0x80){
            // Length Counter 初始化
            if (sound->wave.lenCnt == 0) sound->wave.lenCnt = 256;
            // DAC
            if (sound->wave.output) sound->wave._enable = 1;
            // CurPos 初始化
			sound->wave.__samplePos = 0;
			sound->wave.__freqTimer = (2048 - sound->wave.freq) / 2;
        }
        break;

    /* Channel 4 */
    case 0xFF20:
        sound->noise.lenCnt = 64 - (val & 0x3F);
        break;
    case 0xFF21:
        sound->noise.envInit = val >> 4;
        sound->noise.envDir = (val & 0x8 ? 1 : 0);
        sound->noise.envPeriod = val & 0x7;
		if (!sound->noise.envInit && !sound->noise.envDir) sound->noise._enable = 0;
        break;
    case 0xFF22:
        sound->noise.freqShift = val >> 4;
        sound->noise.freqWidth = (val & 0x8 ? 1 : 0);
        sound->noise.freqRatio = val & 0x7;
        //sound->noise.__freq = (sound->noise.freqRatio ?
        //                       (65536 / sound->noise.freqRatio) >> (sound->noise.freqShift + 1) :
        //                       (65536 * 2) >> (sound->noise.freqShift + 1));
        break;
    case 0xFF23:
        sound->noise.lenCntEn = (val & 0x40 ? 1 : 0);
        if (val & 0x80){
            // Length Counter 初始化
            if (sound->noise.lenCnt == 0) sound->noise.lenCnt = 64;
            // Envelope 初始化
            sound->noise.__envCnt = sound->noise.envPeriod; // 0-15
            sound->noise.__vol = sound->noise.envInit;
            // DAC
			if (!sound->noise.envInit && !sound->noise.envDir) sound->noise._enable = 0;
            else sound->noise._enable=1;
        }
        break;

    /* Control */
    case 0xFF24:
        sound->ctrl.vinRight = (val & 0x80 ? 1 : 0);
        sound->ctrl.volRight = (val & 0x30) >> 4;
        sound->ctrl.vinLeft = (val & 0x8 ? 1 : 0);
        sound->ctrl.volLeft = (val & 0x3);
        break;
    case 0xFF25:
        sound->ctrl.ch4Right = (val & 0x80 ? 1 : 0);
        sound->ctrl.ch3Right = (val & 0x40 ? 1 : 0);
        sound->ctrl.ch2Right = (val & 0x20 ? 1 : 0);
        sound->ctrl.ch1Right = (val & 0x10 ? 1 : 0);
        sound->ctrl.ch4Left = (val & 0x8 ? 1 : 0);
        sound->ctrl.ch3Left = (val & 0x4 ? 1 : 0);
        sound->ctrl.ch2Left = (val & 0x2 ? 1 : 0);
        sound->ctrl.ch1Left = (val & 0x1 ? 1 : 0);
        break;
    case 0xFF26:
        sound->ctrl.switchAll = (val & 0x80 ? 1 : 0);
        break;
    default:
        break;
    }
}