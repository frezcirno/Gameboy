#include "GB_gameboy.h"
#include "GB_memory.h"
#include "GB_insn.h"
#include "GB_cpu.h"

const int insnClock[0x100] = {
  1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
  0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
  2,3,2,2,1,1,2,1,2,2,2,2,1,1,2,1,
  2,3,2,2,3,3,3,1,2,2,2,2,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  2,2,2,2,2,2,1,2,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
  2,3,3,4,3,4,2,4,2,4,3,0,3,6,2,4,
  2,3,3,0,3,4,2,4,2,4,3,0,3,0,2,4,
  3,3,2,0,0,4,2,4,4,1,4,0,0,0,2,4,
  3,3,2,1,0,4,2,4,3,2,4,1,0,0,2,4
};

const int cbClock[0x100] = {
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
  2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
  2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
  2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
  2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2
};

const insnType insnMap[0x100] = {
	/*0x00*/ NOP, MOVBC_XX, STOBC_A, INC_BC, INC_B, DEC_B, MOVB_X, RLCA,
	/*0x08*/ STOXX_SP, ADDHL_BC, LDA_BC, DEC_BC, INC_C, DEC_C, MOVC_X, RRCA,
	/*0x10*/ STOP, MOVDE_XX, STODE_A, INC_DE, INC_D, DEC_D, MOVD_X, RLA,
	/*0x18*/ JR, ADDHL_DE, LDA_DE, DEC_DE, INC_E, DEC_E, MOVE_X, RRA,
	/*0x20*/ JRNZ, MOVHL_XX, MOVPI_A, INC_HL, INC_H, DEC_H, MOVH_X, DAA,
	/*0x28*/ JRZO, ADDHL_HL, MOVA_PI, DEC_HL, INC_L, DEC_L, MOVL_X, CPL,
	/*0x30*/ JRNC, MOVSP_XX, MOVPD_A, INC_SP, INC_P, DEC_P, MOVP_X, SCF,
	/*0x38*/ JRCA, ADDHL_SP, MOVA_PD, DEC_SP, INC_A, DEC_A, MOVA_X, CCF,
	/*0x40*/ MOVB_B, MOVB_C, MOVB_D, MOVB_E, MOVB_H, MOVB_L, MOVB_P, MOVB_A,
	/*0x48*/ MOVC_B, MOVC_C, MOVC_D, MOVC_E, MOVC_H, MOVC_L, MOVC_P, MOVC_A,
	/*0x50*/ MOVD_B, MOVD_C, MOVD_D, MOVD_E, MOVD_H, MOVD_L, MOVD_P, MOVD_A,
	/*0x58*/ MOVE_B, MOVE_C, MOVE_D, MOVE_E, MOVE_H, MOVE_L, MOVE_P, MOVE_A,
	/*0x60*/ MOVH_B, MOVH_C, MOVH_D, MOVH_E, MOVH_H, MOVH_L, MOVH_P, MOVH_A,
	/*0x68*/ MOVL_B, MOVL_C, MOVL_D, MOVL_E, MOVL_H, MOVL_L, MOVL_P, MOVL_A,
	/*0x70*/ MOVP_B, MOVP_C, MOVP_D, MOVP_E, MOVP_H, MOVP_L, HALT, MOVP_A,
	/*0x78*/ MOVA_B, MOVA_C, MOVA_D, MOVA_E, MOVA_H, MOVA_L, MOVA_P, MOVA_A,
	/*0x80*/ ADD_B, ADD_C, ADD_D, ADD_E, ADD_H, ADD_L, ADD_P, ADD_A,
	/*0x88*/ ADC_B, ADC_C, ADC_D, ADC_E, ADC_H, ADC_L, ADC_P, ADC_A,
	/*0x90*/ SUB_B, SUB_C, SUB_D, SUB_E, SUB_H, SUB_L, SUB_P, SUB_A,
	/*0x98*/ SBC_B, SBC_C, SBC_D, SBC_E, SBC_H, SBC_L, SBC_P, SBC_A,
	/*0xA0*/ AND_B, AND_C, AND_D, AND_E, AND_H, AND_L, AND_P, AND_A,
	/*0xA8*/ XOR_B, XOR_C, XOR_D, XOR_E, XOR_H, XOR_L, XOR_P, XOR_A,
	/*0xB0*/ OR_B, OR_C, OR_D, OR_E, OR_H, OR_L, OR_P, OR_A,
	/*0xB8*/ CP_B, CP_C, CP_D, CP_E, CP_H, CP_L, CP_P, CP_A,
	/*0xC0*/ RETNZ, POPBC, JPNZ, JP, CALLNZ, PUSHBC, ADD_X, RST00,
	/*0xC8*/ RETZO, RET, JPZO, CB, CALLZO, CALL, ADC_X, RST08,
	/*0xD0*/ RETNC, POPDE, JPNC, NULL, CALLNC, PUSHDE, SUB_X, RST10,
	/*0xD8*/ RETCA, RETI, JPCA, NULL, CALLCA, NULL, SBC_X, RST18,
	/*0xE0*/ STOEX_A, POPHL, STOEC_A, NULL, NULL, PUSHHL, AND_X, RST20,
	/*0xE8*/ ADDSP_X, JPHL, STOXX_A, NULL, NULL, NULL, XOR_X, RST28,
	/*0xF0*/ LDA_EX, POPAF, LDA_EC, DI, NULL, PUSHAF, OR_X, RST30,
	/*0xF8*/ MOVHL_SPn, MOVSP_HL, LDA_XX, EI, NULL, NULL, CP_X, RST38
};

const insnType cbMap[0x100] = {
	/*0x00*/ RLC_B, RLC_C, RLC_D, RLC_E, RLC_H, RLC_L, RLC_P, RLC_A,
	/*0x08*/ RRC_B, RRC_C, RRC_D, RRC_E, RRC_H, RRC_L, RRC_P, RRC_A,
	/*0x10*/ RL_B, RL_C, RL_D, RL_E, RL_H, RL_L, RL_P, RL_A,
	/*0x18*/ RR_B, RR_C, RR_D, RR_E, RR_H, RR_L, RR_P, RR_A,
	/*0x20*/ SL_B, SL_C, SL_D, SL_E, SL_H, SL_L, SL_P, SL_A,
	/*0x28*/ SR_B, SR_C, SR_D, SR_E, SR_H, SR_L, SR_P, SR_A,
	/*0x30*/ SWAP_B, SWAP_C, SWAP_D, SWAP_E, SWAP_H, SWAP_L, SWAP_P, SWAP_A,
	/*0x38*/ SRL_B, SRL_C, SRL_D, SRL_E, SRL_H, SRL_L, SRL_P, SRL_A,
	/*0x40*/ BIT0_B, BIT0_C, BIT0_D, BIT0_E, BIT0_H, BIT0_L, BIT0_P, BIT0_A,
	/*0x48*/ BIT1_B, BIT1_C, BIT1_D, BIT1_E, BIT1_H, BIT1_L, BIT1_P, BIT1_A,
	/*0x50*/ BIT2_B, BIT2_C, BIT2_D, BIT2_E, BIT2_H, BIT2_L, BIT2_P, BIT2_A,
	/*0x58*/ BIT3_B, BIT3_C, BIT3_D, BIT3_E, BIT3_H, BIT3_L, BIT3_P, BIT3_A,
	/*0x60*/ BIT4_B, BIT4_C, BIT4_D, BIT4_E, BIT4_H, BIT4_L, BIT4_P, BIT4_A,
	/*0x68*/ BIT5_B, BIT5_C, BIT5_D, BIT5_E, BIT5_H, BIT5_L, BIT5_P, BIT5_A,
	/*0x70*/ BIT6_B, BIT6_C, BIT6_D, BIT6_E, BIT6_H, BIT6_L, BIT6_P, BIT6_A,
	/*0x78*/ BIT7_B, BIT7_C, BIT7_D, BIT7_E, BIT7_H, BIT7_L, BIT7_P, BIT7_A,
	/*0x80*/ RES0_B, RES0_C, RES0_D, RES0_E, RES0_H, RES0_L, RES0_P, RES0_A,
	/*0x88*/ RES1_B, RES1_C, RES1_D, RES1_E, RES1_H, RES1_L, RES1_P, RES1_A,
	/*0x90*/ RES2_B, RES2_C, RES2_D, RES2_E, RES2_H, RES2_L, RES2_P, RES2_A,
	/*0x98*/ RES3_B, RES3_C, RES3_D, RES3_E, RES3_H, RES3_L, RES3_P, RES3_A,
	/*0xA0*/ RES4_B, RES4_C, RES4_D, RES4_E, RES4_H, RES4_L, RES4_P, RES4_A,
	/*0xA8*/ RES5_B, RES5_C, RES5_D, RES5_E, RES5_H, RES5_L, RES5_P, RES5_A,
	/*0xB0*/ RES6_B, RES6_C, RES6_D, RES6_E, RES6_H, RES6_L, RES6_P, RES6_A,
	/*0xB8*/ RES7_B, RES7_C, RES7_D, RES7_E, RES7_H, RES7_L, RES7_P, RES7_A,
	/*0xC0*/ SET0_B, SET0_C, SET0_D, SET0_E, SET0_H, SET0_L, SET0_P, SET0_A,
	/*0xC8*/ SET1_B, SET1_C, SET1_D, SET1_E, SET1_H, SET1_L, SET1_P, SET1_A,
	/*0xD0*/ SET2_B, SET2_C, SET2_D, SET2_E, SET2_H, SET2_L, SET2_P, SET2_A,
	/*0xD8*/ SET3_B, SET3_C, SET3_D, SET3_E, SET3_H, SET3_L, SET3_P, SET3_A,
	/*0xE0*/ SET4_B, SET4_C, SET4_D, SET4_E, SET4_H, SET4_L, SET4_P, SET4_A,
	/*0xE8*/ SET5_B, SET5_C, SET5_D, SET5_E, SET5_H, SET5_L, SET5_P, SET5_A,
	/*0xF0*/ SET6_B, SET6_C, SET6_D, SET6_E, SET6_H, SET6_L, SET6_P, SET6_A,
	/*0xF8*/ SET7_B, SET7_C, SET7_D, SET7_E, SET7_H, SET7_L, SET7_P, SET7_A
};

int cpu_init(GB_CPU* cpu) {
	cpu_reset(cpu);
	return 0;
}

void cpu_free(GB_CPU* cpu) {

}

int cpu_reset(GB_CPU* cpu) {
	cpu->AF = 0x01B0;
	cpu->BC = 0x0013;
	cpu->DE = 0x00D8;
	cpu->HL = 0x014D;
	cpu->SP = 0xFFFE;
	cpu->PC = 0x101;
	cpu->insnCycles = 0;
	cpu->intMstEn = 1;
	cpu->isHalt = 0;
	return 0;
}

int cpu_step(GB_CPU* cpu) {
	GB_BYTE intNo = checkInt(cpu);
	if (intNo && cpu->isHalt) {
		cpu->isHalt = 0;
		cpu->PC++;
	}
	if (intNo && cpu->intMstEn) {
		cpu->parent->mem->intFlag &= ~intNo;
		if (intNo & 1) INT(cpu, 0x40);
		else if (intNo & 2) INT(cpu, 0x48);
		else if (intNo & 4) INT(cpu, 0x50);
		else if (intNo & 8) INT(cpu, 0x58);
		else if (intNo & 16) INT(cpu, 0x60);
	}
	else {
		GB_BYTE op = readByte(cpu->parent->mem, cpu->PC++);
		cpu->insnCycles = insnClock[op];
		insnMap[op](cpu); // 可能修改insnT
	}
	return cpu->insnCycles;
}

GB_BYTE checkInt(GB_CPU* cpu) {
	GB_BYTE intNo = cpu->parent->mem->intEnMask & cpu->parent->mem->intFlag; // 有中断 && 此中断未关闭
	if (intNo) {
		if (intNo & 1) return 1;
		else if (intNo & 2) return 2;
		else if (intNo & 4) return 4;
		else if (intNo & 8) return 8;
		else if (intNo & 16) return 16;
	}
	return 0;
}
