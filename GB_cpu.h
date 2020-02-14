#pragma once
#include "GB_common.h"

typedef struct GB_CPU
{
	struct GB_GAMEBOY* parent;
	union {
		GB_WORD AF;
		struct { GB_BYTE F, A; };
	};
	union {
		GB_WORD BC;
		struct { GB_BYTE C, B; };
	};
	union {
		GB_WORD DE;
		struct { GB_BYTE E, D; };
	};
	union {
		GB_WORD HL;
		struct { GB_BYTE L, H; };
	};
	GB_WORD SP, PC;
	int insnCycles;
	int intMstEn; // 软件控制 中断总开关
	int isHalt;
}GB_CPU;

typedef void (*insnType)(GB_CPU*);

#define getFlagX(cpu, mask) (cpu->F & mask)
#define setFlagAnd(cpu, mask) (cpu->F &= mask)
#define setFlagOr(cpu, mask) (cpu->F |= mask)

#define FLAG_ZERO(cpu) (getFlagX(cpu, 0x80) >> 7)
#define FLAG_SUBS(cpu) (getFlagX(cpu, 0x40) >> 6)
#define FLAG_HACA(cpu) (getFlagX(cpu, 0x20) >> 5)
#define FLAG_CARR(cpu) (getFlagX(cpu, 0x10) >> 4)

#define SET_FLAG_ZERO(cpu) (setFlagOr(cpu, 0x80))
#define SET_FLAG_SUBS(cpu) (setFlagOr(cpu, 0x40))
#define SET_FLAG_HACA(cpu) (setFlagOr(cpu, 0x20))
#define SET_FLAG_CARR(cpu) (setFlagOr(cpu, 0x10))

#define RST_FLAG_ZERO(cpu) (setFlagAnd(cpu, 0x7F))
#define RST_FLAG_SUBS(cpu) (setFlagAnd(cpu, 0xBF))
#define RST_FLAG_HACA(cpu) (setFlagAnd(cpu, 0xDF))
#define RST_FLAG_CARR(cpu) (setFlagAnd(cpu, 0xEF))

int cpu_init(GB_CPU* cpu);
void cpu_free(GB_CPU* cpu);

int cpu_reset(GB_CPU* cpu);

int cpu_step(GB_CPU* cpu);

GB_BYTE checkInt(GB_CPU* cpu);