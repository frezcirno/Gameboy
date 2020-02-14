#pragma once
#include "GB_cpu.h"

/* CB PREFIX */
extern const insnType cbMap[0x100];
extern const int cbClock[0x100];

void CB(GB_CPU* cpu) {
	GB_BYTE op = readByte(cpu->parent->mem, cpu->PC++);
	cbMap[op](cpu);
	cpu->insnCycles = cbClock[op]; // fix time
}

/* UTILS */
#define DEFERHL(FOO)        \
    GB_BYTE defer = readByte(cpu->parent->mem, cpu->HL);    \
    FOO(defer);            \
    writeByte(cpu->parent->mem, cpu->HL, defer)

/* REG -> REG */
void MOVA_A(GB_CPU* cpu) { }
void MOVA_F(GB_CPU* cpu) { cpu->A = cpu->F; }
void MOVA_B(GB_CPU* cpu) { cpu->A = cpu->B; }
void MOVA_C(GB_CPU* cpu) { cpu->A = cpu->C; }
void MOVA_D(GB_CPU* cpu) { cpu->A = cpu->D; }
void MOVA_E(GB_CPU* cpu) { cpu->A = cpu->E; }
void MOVA_H(GB_CPU* cpu) { cpu->A = cpu->H; }
void MOVA_L(GB_CPU* cpu) { cpu->A = cpu->L; }

void MOVB_A(GB_CPU* cpu) { cpu->B = cpu->A; }
void MOVB_F(GB_CPU* cpu) { cpu->B = cpu->F; }
void MOVB_B(GB_CPU* cpu) { }
void MOVB_C(GB_CPU* cpu) { cpu->B = cpu->C; }
void MOVB_D(GB_CPU* cpu) { cpu->B = cpu->D; }
void MOVB_E(GB_CPU* cpu) { cpu->B = cpu->E; }
void MOVB_H(GB_CPU* cpu) { cpu->B = cpu->H; }
void MOVB_L(GB_CPU* cpu) { cpu->B = cpu->L; }

void MOVC_A(GB_CPU* cpu) { cpu->C = cpu->A; }
void MOVC_F(GB_CPU* cpu) { cpu->C = cpu->F; }
void MOVC_B(GB_CPU* cpu) { cpu->C = cpu->B; }
void MOVC_C(GB_CPU* cpu) { }
void MOVC_D(GB_CPU* cpu) { cpu->C = cpu->D; }
void MOVC_E(GB_CPU* cpu) { cpu->C = cpu->E; }
void MOVC_H(GB_CPU* cpu) { cpu->C = cpu->H; }
void MOVC_L(GB_CPU* cpu) { cpu->C = cpu->L; }

void MOVD_A(GB_CPU* cpu) { cpu->D = cpu->A; }
void MOVD_F(GB_CPU* cpu) { cpu->D = cpu->F; }
void MOVD_B(GB_CPU* cpu) { cpu->D = cpu->B; }
void MOVD_C(GB_CPU* cpu) { cpu->D = cpu->C; }
void MOVD_D(GB_CPU* cpu) { }
void MOVD_E(GB_CPU* cpu) { cpu->D = cpu->E; }
void MOVD_H(GB_CPU* cpu) { cpu->D = cpu->H; }
void MOVD_L(GB_CPU* cpu) { cpu->D = cpu->L; }

void MOVE_A(GB_CPU* cpu) { cpu->E = cpu->A; }
void MOVE_F(GB_CPU* cpu) { cpu->E = cpu->F; }
void MOVE_B(GB_CPU* cpu) { cpu->E = cpu->B; }
void MOVE_C(GB_CPU* cpu) { cpu->E = cpu->C; }
void MOVE_D(GB_CPU* cpu) { cpu->E = cpu->D; }
void MOVE_E(GB_CPU* cpu) { }
void MOVE_H(GB_CPU* cpu) { cpu->E = cpu->H; }
void MOVE_L(GB_CPU* cpu) { cpu->E = cpu->L; }

void MOVH_A(GB_CPU* cpu) { cpu->H = cpu->A; }
void MOVH_F(GB_CPU* cpu) { cpu->H = cpu->F; }
void MOVH_B(GB_CPU* cpu) { cpu->H = cpu->B; }
void MOVH_C(GB_CPU* cpu) { cpu->H = cpu->C; }
void MOVH_D(GB_CPU* cpu) { cpu->H = cpu->D; }
void MOVH_E(GB_CPU* cpu) { cpu->H = cpu->E; }
void MOVH_H(GB_CPU* cpu) { }
void MOVH_L(GB_CPU* cpu) { cpu->H = cpu->L; }

void MOVL_A(GB_CPU* cpu) { cpu->L = cpu->A; }
void MOVL_F(GB_CPU* cpu) { cpu->L = cpu->F; }
void MOVL_B(GB_CPU* cpu) { cpu->L = cpu->B; }
void MOVL_C(GB_CPU* cpu) { cpu->L = cpu->C; }
void MOVL_D(GB_CPU* cpu) { cpu->L = cpu->D; }
void MOVL_E(GB_CPU* cpu) { cpu->L = cpu->E; }
void MOVL_H(GB_CPU* cpu) { cpu->L = cpu->H; }
void MOVL_L(GB_CPU* cpu) { }

void MOVA_X(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, cpu->PC++); }
void MOVB_X(GB_CPU* cpu) { cpu->B = readByte(cpu->parent->mem, cpu->PC++); }
void MOVC_X(GB_CPU* cpu) { cpu->C = readByte(cpu->parent->mem, cpu->PC++); }
void MOVD_X(GB_CPU* cpu) { cpu->D = readByte(cpu->parent->mem, cpu->PC++); }
void MOVE_X(GB_CPU* cpu) { cpu->E = readByte(cpu->parent->mem, cpu->PC++); }
void MOVH_X(GB_CPU* cpu) { cpu->H = readByte(cpu->parent->mem, cpu->PC++); }
void MOVL_X(GB_CPU* cpu) { cpu->L = readByte(cpu->parent->mem, cpu->PC++); }

void MOVBC_XX(GB_CPU* cpu) { cpu->BC = readWord(cpu->parent->mem, cpu->PC);  cpu->PC += 2; }
void MOVDE_XX(GB_CPU* cpu) { cpu->DE = readWord(cpu->parent->mem, cpu->PC);  cpu->PC += 2; }
void MOVHL_XX(GB_CPU* cpu) { cpu->HL = readWord(cpu->parent->mem, cpu->PC);  cpu->PC += 2; }
void MOVSP_XX(GB_CPU* cpu) { cpu->SP = readWord(cpu->parent->mem, cpu->PC);  cpu->PC += 2; }

void MOVSP_HL(GB_CPU* cpu) { cpu->SP = cpu->HL; }
void MOVHL_SPn(GB_CPU* cpu) {
    GB_SBYTE offset8 = (GB_SBYTE)readByte(cpu->parent->mem, cpu->PC++);
    cpu->HL = cpu->SP + offset8;
    cpu->F &= 0b00111111;
	if (((cpu->SP & 0xF) + (offset8 & 0xF)) > 0xF) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);
	if (((cpu->SP & 0xFF) + (offset8 & 0xFF)) > 0xFF) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);
}


/* MEM -> REG */
void LDA_BC(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, cpu->BC); }
void LDA_DE(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, cpu->DE); }

void MOVA_P(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, cpu->HL); }
void MOVB_P(GB_CPU* cpu) { cpu->B = readByte(cpu->parent->mem, cpu->HL); }
void MOVC_P(GB_CPU* cpu) { cpu->C = readByte(cpu->parent->mem, cpu->HL); }
void MOVD_P(GB_CPU* cpu) { cpu->D = readByte(cpu->parent->mem, cpu->HL); }
void MOVE_P(GB_CPU* cpu) { cpu->E = readByte(cpu->parent->mem, cpu->HL); }
void MOVH_P(GB_CPU* cpu) { cpu->H = readByte(cpu->parent->mem, cpu->HL); }
void MOVL_P(GB_CPU* cpu) { cpu->L = readByte(cpu->parent->mem, cpu->HL); }

void MOVA_PD(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, cpu->HL--); }
void MOVA_PI(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, cpu->HL++); }

void LDA_EX(GB_CPU* cpu) {
    GB_BYTE addr8 = readByte(cpu->parent->mem, cpu->PC++);
    cpu->A = readByte(cpu->parent->mem, 0xFF00 | addr8);
}

void LDA_XX(GB_CPU* cpu) {
    GB_WORD addr16 = readWord(cpu->parent->mem, cpu->PC); cpu->PC += 2;
    cpu->A = readByte(cpu->parent->mem, addr16);
}

void LDB_BC(GB_CPU* cpu) { cpu->B = readByte(cpu->parent->mem, cpu->BC); }
void LDB_DE(GB_CPU* cpu) { cpu->B = readByte(cpu->parent->mem, cpu->DE); }

void LDC_BC(GB_CPU* cpu) { cpu->C = readByte(cpu->parent->mem, cpu->BC); }
void LDC_DE(GB_CPU* cpu) { cpu->C = readByte(cpu->parent->mem, cpu->DE); }

void LDD_BC(GB_CPU* cpu) { cpu->D = readByte(cpu->parent->mem, cpu->BC); }
void LDD_DE(GB_CPU* cpu) { cpu->D = readByte(cpu->parent->mem, cpu->DE); }

void LDE_BC(GB_CPU* cpu) { cpu->E = readByte(cpu->parent->mem, cpu->BC); }
void LDE_DE(GB_CPU* cpu) { cpu->E = readByte(cpu->parent->mem, cpu->DE); }

void LDA_EC(GB_CPU* cpu) { cpu->A = readByte(cpu->parent->mem, 0xFF00 | cpu->C); }


/* REG -> MEM */
void STOBC_A(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->BC, cpu->A); }
void STODE_A(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->DE, cpu->A); }
void MOVP_A(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->A); }

void MOVPD_A(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL--, cpu->A); }
void MOVPI_A(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL++, cpu->A); }

void STOEX_A(GB_CPU* cpu) {
    GB_BYTE addr8 = readByte(cpu->parent->mem, cpu->PC++);
    writeByte(cpu->parent->mem, 0xFF00 | addr8, cpu->A);
}

void STOXX_A(GB_CPU* cpu) {
    GB_WORD addr16 = readWord(cpu->parent->mem, cpu->PC); cpu->PC += 2;
    writeByte(cpu->parent->mem, addr16, cpu->A);
}

void STOXX_SP(GB_CPU* cpu) {
    GB_WORD addr16 = readWord(cpu->parent->mem, cpu->PC); cpu->PC += 2;
    writeWord(cpu->parent->mem, addr16, cpu->SP);
}

void MOVP_B(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->B); }
void MOVP_C(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->C); }
void MOVP_D(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->D); }
void MOVP_E(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->E); }
void MOVP_H(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->H); }
void MOVP_L(GB_CPU* cpu) { writeByte(cpu->parent->mem, cpu->HL, cpu->L); }
void MOVP_X(GB_CPU* cpu) {
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++);
    writeByte(cpu->parent->mem, cpu->HL, op8);
}

void STOEC_A(GB_CPU* cpu) {
    writeByte(cpu->parent->mem, 0xFF00 | cpu->C, cpu->A);
}

/* STACK */
#define PUSH(OP)                                     \
    cpu->SP -= 2;                                \
    writeWord(cpu->parent->mem, cpu->SP, OP)

#define POP(OP)                                        \
    (OP) = readWord(cpu->parent->mem, cpu->SP);    \
    cpu->SP += 2

void PUSHAF(GB_CPU* cpu) { PUSH(cpu->AF);}
void PUSHBC(GB_CPU* cpu) { PUSH(cpu->BC); }
void PUSHDE(GB_CPU* cpu) { PUSH(cpu->DE); }
void PUSHHL(GB_CPU* cpu) { PUSH(cpu->HL); }

void POPAF(GB_CPU* cpu) { POP(cpu->AF); cpu->F &= 0xF0; }
void POPBC(GB_CPU* cpu) { POP(cpu->BC); }
void POPDE(GB_CPU* cpu) { POP(cpu->DE); }
void POPHL(GB_CPU* cpu) { POP(cpu->HL); }

/* ARITHMETIC-8 */
#define ADD(OP1, OP2, CARRY)                                                \
    RST_FLAG_SUBS(cpu);                                                        \
    if ((OP1 & 0xF) + (OP2 & 0xF) + (CARRY) > 0xF) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);        \
    if ((OP1 & 0xFF) + (OP2 & 0xFF) + (CARRY) > 0xFF) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);		\
    OP1 += OP2 + CARRY;                                                        \
    if (OP1 == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu)

#define SUB(OP1, OP2, BORROW)                                                \
    SET_FLAG_SUBS(cpu);                                                        \
    if ((OP1 & 0xF) - (OP2 & 0xF) - (BORROW) < 0) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);    \
    if ((OP1 & 0xFF) - (OP2 & 0xFF) - (BORROW) < 0) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);  \
    OP1 -= OP2 + BORROW;                                                    \
    if (OP1 == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu)

void ADD_A(GB_CPU* cpu) { ADD(cpu->A, cpu->A, 0); }
void ADD_B(GB_CPU* cpu) { ADD(cpu->A, cpu->B, 0); }
void ADD_C(GB_CPU* cpu) { ADD(cpu->A, cpu->C, 0); }
void ADD_D(GB_CPU* cpu) { ADD(cpu->A, cpu->D, 0); }
void ADD_E(GB_CPU* cpu) { ADD(cpu->A, cpu->E, 0); }
void ADD_H(GB_CPU* cpu) { ADD(cpu->A, cpu->H, 0); }
void ADD_L(GB_CPU* cpu) { ADD(cpu->A, cpu->L, 0); }
void ADC_A(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->A, carry); }
void ADC_B(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->B, carry); }
void ADC_C(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->C, carry); }
void ADC_D(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->D, carry); }
void ADC_E(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->E, carry); }
void ADC_H(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->H, carry); }
void ADC_L(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); ADD(cpu->A, cpu->L, carry); }

void SUB_A(GB_CPU* cpu) { SUB(cpu->A, cpu->A, 0); }
void SUB_B(GB_CPU* cpu) { SUB(cpu->A, cpu->B, 0); }
void SUB_C(GB_CPU* cpu) { SUB(cpu->A, cpu->C, 0); }
void SUB_D(GB_CPU* cpu) { SUB(cpu->A, cpu->D, 0); }
void SUB_E(GB_CPU* cpu) { SUB(cpu->A, cpu->E, 0); }
void SUB_H(GB_CPU* cpu) { SUB(cpu->A, cpu->H, 0); }
void SUB_L(GB_CPU* cpu) { SUB(cpu->A, cpu->L, 0); }
void SBC_A(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->A, carry); }
void SBC_B(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->B, carry); }
void SBC_C(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->C, carry); }
void SBC_D(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->D, carry); }
void SBC_E(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->E, carry); }
void SBC_H(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->H, carry); }
void SBC_L(GB_CPU* cpu) { GB_BYTE carry = FLAG_CARR(cpu); SUB(cpu->A, cpu->L, carry); }

void ADD_P(GB_CPU* cpu) {
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL);
    ADD(cpu->A, op8, 0);
}
void ADD_X(GB_CPU* cpu) {
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++);
    ADD(cpu->A, op8, 0);
}
void ADC_P(GB_CPU* cpu) {
    GB_BYTE carry = FLAG_CARR(cpu);
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL);
    ADD(cpu->A, op8, carry);
}
void ADC_X(GB_CPU* cpu) {
    GB_BYTE carry = FLAG_CARR(cpu);
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++);
    ADD(cpu->A, op8, carry);
}

void SUB_P(GB_CPU* cpu) {
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL);
    SUB(cpu->A, op8, 0);
}
void SUB_X(GB_CPU* cpu) {
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++);
    SUB(cpu->A, op8, 0);
}
void SBC_P(GB_CPU* cpu) {
    GB_BYTE carry = FLAG_CARR(cpu);
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL);
    SUB(cpu->A, op8, carry);
}
void SBC_X(GB_CPU* cpu) {
    GB_BYTE carry = FLAG_CARR(cpu);
    GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++);
    SUB(cpu->A, op8, carry);
}

/* LOGIC */
#define AND(OP1, OP2)    \
    OP1 &= OP2;          \
    if (OP1 == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);    \
    cpu->F &= 0b10101111;    /* SUBS & CARR */                \
    cpu->F |= 0b00100000    /* HACA */

#define OR(OP1, OP2)     \
    OP1 |= OP2;          \
    if (OP1 == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);    \
    cpu->F &= 0b10001111

#define XOR(OP1, OP2)    \
    OP1 ^= OP2;          \
    if (OP1 == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);    \
    cpu->F &= 0b10001111

void AND_A(GB_CPU* cpu) { AND(cpu->A, cpu->A); }
void AND_B(GB_CPU* cpu) { AND(cpu->A, cpu->B); }
void AND_C(GB_CPU* cpu) { AND(cpu->A, cpu->C); }
void AND_D(GB_CPU* cpu) { AND(cpu->A, cpu->D); }
void AND_E(GB_CPU* cpu) { AND(cpu->A, cpu->E); }
void AND_H(GB_CPU* cpu) { AND(cpu->A, cpu->H); }
void AND_L(GB_CPU* cpu) { AND(cpu->A, cpu->L); }
void AND_P(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL); AND(cpu->A, op8); }
void AND_X(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++); AND(cpu->A, op8); }

void OR_A(GB_CPU* cpu) { OR(cpu->A, cpu->A); }
void OR_B(GB_CPU* cpu) { OR(cpu->A, cpu->B); }
void OR_C(GB_CPU* cpu) { OR(cpu->A, cpu->C); }
void OR_D(GB_CPU* cpu) { OR(cpu->A, cpu->D); }
void OR_E(GB_CPU* cpu) { OR(cpu->A, cpu->E); }
void OR_H(GB_CPU* cpu) { OR(cpu->A, cpu->H); }
void OR_L(GB_CPU* cpu) { OR(cpu->A, cpu->L); }
void OR_P(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL); OR(cpu->A, op8); }
void OR_X(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++); OR(cpu->A, op8); }

void XOR_A(GB_CPU* cpu) { XOR(cpu->A, cpu->A); }
void XOR_B(GB_CPU* cpu) { XOR(cpu->A, cpu->B); }
void XOR_C(GB_CPU* cpu) { XOR(cpu->A, cpu->C); }
void XOR_D(GB_CPU* cpu) { XOR(cpu->A, cpu->D); }
void XOR_E(GB_CPU* cpu) { XOR(cpu->A, cpu->E); }
void XOR_H(GB_CPU* cpu) { XOR(cpu->A, cpu->H); }
void XOR_L(GB_CPU* cpu) { XOR(cpu->A, cpu->L); }
void XOR_P(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL); XOR(cpu->A, op8); }
void XOR_X(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++); XOR(cpu->A, op8); }

/* COMPARE */
#define CP(OP1, OP2)                                                        \
    if (OP1 == OP2) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);            \
    SET_FLAG_SUBS(cpu);                                                     \
    if ((OP1 & 0xF) - (OP2 & 0xF) < 0) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);    \
    if ((OP1 & 0xFF) - (OP2 & 0xFF) < 0) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu)

void CP_A(GB_CPU* cpu) { CP(cpu->A, cpu->A); }
void CP_B(GB_CPU* cpu) { CP(cpu->A, cpu->B); }
void CP_C(GB_CPU* cpu) { CP(cpu->A, cpu->C); }
void CP_D(GB_CPU* cpu) { CP(cpu->A, cpu->D); }
void CP_E(GB_CPU* cpu) { CP(cpu->A, cpu->E); }
void CP_H(GB_CPU* cpu) { CP(cpu->A, cpu->H); }
void CP_L(GB_CPU* cpu) { CP(cpu->A, cpu->L); }
void CP_P(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->HL); CP(cpu->A, op8); }
void CP_X(GB_CPU* cpu) { GB_BYTE op8 = readByte(cpu->parent->mem, cpu->PC++); CP(cpu->A, op8); }

/* INC & DEC */
#define INC(OP)            \
    if (((OP) & 0xF) + (1) > 0xF) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);    \
    RST_FLAG_SUBS(cpu);    \
    OP++;                  \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu)

#define DEC(OP)            \
    if (((OP) & 0xF) - (1) < 0) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);    \
    SET_FLAG_SUBS(cpu);    \
    OP--;                  \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu)

void INC_A(GB_CPU* cpu) { INC(cpu->A); }
void INC_B(GB_CPU* cpu) { INC(cpu->B); }
void INC_C(GB_CPU* cpu) { INC(cpu->C); }
void INC_D(GB_CPU* cpu) { INC(cpu->D); }
void INC_E(GB_CPU* cpu) { INC(cpu->E); }
void INC_H(GB_CPU* cpu) { INC(cpu->H); }
void INC_L(GB_CPU* cpu) { INC(cpu->L); }
void INC_P(GB_CPU* cpu) { DEFERHL(INC); }

void DEC_A(GB_CPU* cpu) { DEC(cpu->A); }
void DEC_B(GB_CPU* cpu) { DEC(cpu->B); }
void DEC_C(GB_CPU* cpu) { DEC(cpu->C); }
void DEC_D(GB_CPU* cpu) { DEC(cpu->D); }
void DEC_E(GB_CPU* cpu) { DEC(cpu->E); }
void DEC_H(GB_CPU* cpu) { DEC(cpu->H); }
void DEC_L(GB_CPU* cpu) { DEC(cpu->L); }
void DEC_P(GB_CPU* cpu) { DEFERHL(DEC); }

/* ARITHMETIC-16 */
#define ADD16(OP1, OP2)         \
    RST_FLAG_SUBS(cpu);         \
    if ((OP1 & 0xFFF) + (OP2 & 0xFFF) > 0xFFF) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);        \
    if ((OP1 & 0xFFFF) + (OP2 & 0xFFFF) > 0xFFFF) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);	 \
    OP1 += OP2

void ADDHL_BC(GB_CPU* cpu) { ADD16(cpu->HL, cpu->BC); }
void ADDHL_DE(GB_CPU* cpu) { ADD16(cpu->HL, cpu->DE); }
void ADDHL_HL(GB_CPU* cpu) { ADD16(cpu->HL, cpu->HL); }
void ADDHL_SP(GB_CPU* cpu) { ADD16(cpu->HL, cpu->SP); }
void ADDSP_X(GB_CPU* cpu) {
    GB_SBYTE op8 = (GB_SBYTE)readByte(cpu->parent->mem, cpu->PC++);
	cpu->F &= 0b00111111;
	if ((cpu->SP & 0xF) + (op8 & 0xF) > 0xF) SET_FLAG_HACA(cpu); else RST_FLAG_HACA(cpu);
	if ((cpu->SP & 0xFF) + (op8 & 0xFF) > 0xFF) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);
	cpu->SP += op8;
}

void INC_BC(GB_CPU* cpu) { cpu->BC++; }
void INC_DE(GB_CPU* cpu) { cpu->DE++; }
void INC_HL(GB_CPU* cpu) { cpu->HL++; }
void INC_SP(GB_CPU* cpu) { cpu->SP++; }

void DEC_BC(GB_CPU* cpu) { cpu->BC--; }
void DEC_DE(GB_CPU* cpu) { cpu->DE--; }
void DEC_HL(GB_CPU* cpu) { cpu->HL--; }
void DEC_SP(GB_CPU* cpu) { cpu->SP--; }

/* Miscellaneous */
#define SWAP(OP)                \
    (OP) = ((OP) << 4 | (OP) >> 4);    \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);    \
    cpu->F &= 0b10001111

void SWAP_A(GB_CPU* cpu) { SWAP(cpu->A); }
void SWAP_B(GB_CPU* cpu) { SWAP(cpu->B); }
void SWAP_C(GB_CPU* cpu) { SWAP(cpu->C); }
void SWAP_D(GB_CPU* cpu) { SWAP(cpu->D); }
void SWAP_E(GB_CPU* cpu) { SWAP(cpu->E); }
void SWAP_H(GB_CPU* cpu) { SWAP(cpu->H); }
void SWAP_L(GB_CPU* cpu) { SWAP(cpu->L); }
void SWAP_P(GB_CPU* cpu) { DEFERHL(SWAP); }

void DAA(GB_CPU* cpu) { /* DAA--进行BCD运算后的调整 */
	GB_BYTE correction = 0;
	if (!FLAG_SUBS(cpu)) {
		if (FLAG_HACA(cpu) || (cpu->A & 0xF) > 0x9) correction += 0x6;
		if (FLAG_CARR(cpu) || (cpu->A & 0xFF) > 0x90) {
			correction += 0x60;
			SET_FLAG_CARR(cpu);
		}
	} else {
		if (FLAG_HACA(cpu)) correction += 0xFA;
		if (FLAG_CARR(cpu)) correction += 0xA0;
	}

	cpu->A += correction;
	if (cpu->A == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);
	RST_FLAG_HACA(cpu);
}

void CPL(GB_CPU* cpu) { cpu->A ^= 0xFF;  cpu->F |= 0b01100000; }
void CCF(GB_CPU* cpu) { cpu->F ^= 0b00010000;  cpu->F &= 0b10011111; }
void SCF(GB_CPU* cpu) { cpu->F |= 0b00010000;  cpu->F &= 0b10011111; }

void NOP(GB_CPU* cpu) { }
void HALT(GB_CPU* cpu) { cpu->PC--; cpu->isHalt = 1; } // Loop
void STOP(GB_CPU* cpu) { cpu->parent->run = 0; }
void DI(GB_CPU* cpu) { cpu->intMstEn = 0; }
void EI(GB_CPU* cpu) { cpu->intMstEn = 1; }

/* ROTATE */
#define RLC(OP)                    \
	GB_BYTE oldOp = (OP);			\
    (OP) = oldOp << 1 | oldOp >> 7;        \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);        \
    if (oldOp >> 7) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);    \
    cpu->F &= 0b10011111

#define RRC(OP)                    \
	GB_BYTE oldOp = (OP);			\
    (OP) = oldOp << 7 | oldOp >> 1;        \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);        \
    if (oldOp & 1) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);    \
    cpu->F &= 0b10011111

#define RL(OP)                \
    GB_BYTE oldOp = (OP);        \
    (OP) = oldOp << 1 | FLAG_CARR(cpu);                            \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);            \
    if (oldOp >> 7) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);    \
    cpu->F &= 0b10011111

#define RR(OP)                \
    GB_BYTE oldOp = (OP);        \
    (OP) = FLAG_CARR(cpu) << 7 | oldOp >> 1;        \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);            \
    if (oldOp & 1) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);    \
    cpu->F &= 0b10011111

void RLCA(GB_CPU* cpu) { RLC(cpu->A); cpu->F &= 0b00011111; }

void RLC_A(GB_CPU* cpu) { RLC(cpu->A); }
void RLC_B(GB_CPU* cpu) { RLC(cpu->B); }
void RLC_C(GB_CPU* cpu) { RLC(cpu->C); }
void RLC_D(GB_CPU* cpu) { RLC(cpu->D); }
void RLC_E(GB_CPU* cpu) { RLC(cpu->E); }
void RLC_H(GB_CPU* cpu) { RLC(cpu->H); }
void RLC_L(GB_CPU* cpu) { RLC(cpu->L); }
void RLC_P(GB_CPU* cpu) { DEFERHL(RLC); }

void RRCA(GB_CPU* cpu) { RRC(cpu->A); cpu->F &= 0b00011111; }

void RRC_A(GB_CPU* cpu) { RRC(cpu->A); }
void RRC_B(GB_CPU* cpu) { RRC(cpu->B); }
void RRC_C(GB_CPU* cpu) { RRC(cpu->C); }
void RRC_D(GB_CPU* cpu) { RRC(cpu->D); }
void RRC_E(GB_CPU* cpu) { RRC(cpu->E); }
void RRC_H(GB_CPU* cpu) { RRC(cpu->H); }
void RRC_L(GB_CPU* cpu) { RRC(cpu->L); }
void RRC_P(GB_CPU* cpu) { DEFERHL(RRC); }

void RLA(GB_CPU* cpu) { RL(cpu->A);	cpu->F &= 0b00011111; }

void RL_A(GB_CPU* cpu) { RL(cpu->A); }
void RL_B(GB_CPU* cpu) { RL(cpu->B); }
void RL_C(GB_CPU* cpu) { RL(cpu->C); }
void RL_D(GB_CPU* cpu) { RL(cpu->D); }
void RL_E(GB_CPU* cpu) { RL(cpu->E); }
void RL_H(GB_CPU* cpu) { RL(cpu->H); }
void RL_L(GB_CPU* cpu) { RL(cpu->L); }
void RL_P(GB_CPU* cpu) { DEFERHL(RL); }

void RRA(GB_CPU* cpu) { RR(cpu->A);	cpu->F &= 0b00011111; }

void RR_A(GB_CPU* cpu) { RR(cpu->A); }
void RR_B(GB_CPU* cpu) { RR(cpu->B); }
void RR_C(GB_CPU* cpu) { RR(cpu->C); }
void RR_D(GB_CPU* cpu) { RR(cpu->D); }
void RR_E(GB_CPU* cpu) { RR(cpu->E); }
void RR_H(GB_CPU* cpu) { RR(cpu->H); }
void RR_L(GB_CPU* cpu) { RR(cpu->L); }
void RR_P(GB_CPU* cpu) { DEFERHL(RR); }

/* SHIFT */
#define SL(OP)        \
    if ((OP) >> 7) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);   \
    (OP) <<= 1;                                                        \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);        \
    cpu->F &= 0b10011111

#define SR(OP)    /* 算数右移 */    \
    if ((OP) & 1) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);    \
    (OP) = (GB_BYTE)((GB_SBYTE)(OP) >> 1);                               \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);        \
    cpu->F &= 0b10011111

#define SRL(OP)    /* 逻辑右移 */    \
    if ((OP) & 1) SET_FLAG_CARR(cpu); else RST_FLAG_CARR(cpu);    \
    (OP) >>= 1;                                                        \
    if ((OP) == 0) SET_FLAG_ZERO(cpu); else RST_FLAG_ZERO(cpu);        \
    cpu->F &= 0b10011111

void SL_A(GB_CPU* cpu) { SL(cpu->A); }
void SL_B(GB_CPU* cpu) { SL(cpu->B); }
void SL_C(GB_CPU* cpu) { SL(cpu->C); }
void SL_D(GB_CPU* cpu) { SL(cpu->D); }
void SL_E(GB_CPU* cpu) { SL(cpu->E); }
void SL_H(GB_CPU* cpu) { SL(cpu->H); }
void SL_L(GB_CPU* cpu) { SL(cpu->L); }
void SL_P(GB_CPU* cpu) { DEFERHL(SL); }

void SR_A(GB_CPU* cpu) { SR(cpu->A); }
void SR_B(GB_CPU* cpu) { SR(cpu->B); }
void SR_C(GB_CPU* cpu) { SR(cpu->C); }
void SR_D(GB_CPU* cpu) { SR(cpu->D); }
void SR_E(GB_CPU* cpu) { SR(cpu->E); }
void SR_H(GB_CPU* cpu) { SR(cpu->H); }
void SR_L(GB_CPU* cpu) { SR(cpu->L); }
void SR_P(GB_CPU* cpu) { DEFERHL(SR); }

void SRL_A(GB_CPU* cpu) { SRL(cpu->A); }
void SRL_B(GB_CPU* cpu) { SRL(cpu->B); }
void SRL_C(GB_CPU* cpu) { SRL(cpu->C); }
void SRL_D(GB_CPU* cpu) { SRL(cpu->D); }
void SRL_E(GB_CPU* cpu) { SRL(cpu->E); }
void SRL_H(GB_CPU* cpu) { SRL(cpu->H); }
void SRL_L(GB_CPU* cpu) { SRL(cpu->L); }
void SRL_P(GB_CPU* cpu) { DEFERHL(SRL); }

/* BIT */
#define BIT(OP, bit)    \
    if (((OP) >> bit) & 1) RST_FLAG_ZERO(cpu); else SET_FLAG_ZERO(cpu);    \
    cpu->F &= 0b10111111;    \
    cpu->F |= 0b00100000

#define SET(OP, bit)    \
    (OP) |= (1 << bit)

#define RES(OP, bit)    \
    (OP) &= (0xFF ^ (1 << bit))

#define BIT0(OP) BIT(OP,0)
#define BIT1(OP) BIT(OP,1)
#define BIT2(OP) BIT(OP,2)
#define BIT3(OP) BIT(OP,3)
#define BIT4(OP) BIT(OP,4)
#define BIT5(OP) BIT(OP,5)
#define BIT6(OP) BIT(OP,6)
#define BIT7(OP) BIT(OP,7)
#define SET0(OP) SET(OP,0)
#define SET1(OP) SET(OP,1)
#define SET2(OP) SET(OP,2)
#define SET3(OP) SET(OP,3)
#define SET4(OP) SET(OP,4)
#define SET5(OP) SET(OP,5)
#define SET6(OP) SET(OP,6)
#define SET7(OP) SET(OP,7)
#define RES0(OP) RES(OP,0)
#define RES1(OP) RES(OP,1)
#define RES2(OP) RES(OP,2)
#define RES3(OP) RES(OP,3)
#define RES4(OP) RES(OP,4)
#define RES5(OP) RES(OP,5)
#define RES6(OP) RES(OP,6)
#define RES7(OP) RES(OP,7)

void BIT0_A(GB_CPU* cpu) { BIT0(cpu->A); }
void BIT0_B(GB_CPU* cpu) { BIT0(cpu->B); }
void BIT0_C(GB_CPU* cpu) { BIT0(cpu->C); }
void BIT0_D(GB_CPU* cpu) { BIT0(cpu->D); }
void BIT0_E(GB_CPU* cpu) { BIT0(cpu->E); }
void BIT0_H(GB_CPU* cpu) { BIT0(cpu->H); }
void BIT0_L(GB_CPU* cpu) { BIT0(cpu->L); }
void BIT0_P(GB_CPU* cpu) { DEFERHL(BIT0); }
void SET0_A(GB_CPU* cpu) { SET0(cpu->A); }
void SET0_B(GB_CPU* cpu) { SET0(cpu->B); }
void SET0_C(GB_CPU* cpu) { SET0(cpu->C); }
void SET0_D(GB_CPU* cpu) { SET0(cpu->D); }
void SET0_E(GB_CPU* cpu) { SET0(cpu->E); }
void SET0_H(GB_CPU* cpu) { SET0(cpu->H); }
void SET0_L(GB_CPU* cpu) { SET0(cpu->L); }
void SET0_P(GB_CPU* cpu) { DEFERHL(SET0); }
void RES0_A(GB_CPU* cpu) { RES0(cpu->A); }
void RES0_B(GB_CPU* cpu) { RES0(cpu->B); }
void RES0_C(GB_CPU* cpu) { RES0(cpu->C); }
void RES0_D(GB_CPU* cpu) { RES0(cpu->D); }
void RES0_E(GB_CPU* cpu) { RES0(cpu->E); }
void RES0_H(GB_CPU* cpu) { RES0(cpu->H); }
void RES0_L(GB_CPU* cpu) { RES0(cpu->L); }
void RES0_P(GB_CPU* cpu) { DEFERHL(RES0); }

void BIT1_A(GB_CPU* cpu) { BIT1(cpu->A); }
void BIT1_B(GB_CPU* cpu) { BIT1(cpu->B); }
void BIT1_C(GB_CPU* cpu) { BIT1(cpu->C); }
void BIT1_D(GB_CPU* cpu) { BIT1(cpu->D); }
void BIT1_E(GB_CPU* cpu) { BIT1(cpu->E); }
void BIT1_H(GB_CPU* cpu) { BIT1(cpu->H); }
void BIT1_L(GB_CPU* cpu) { BIT1(cpu->L); }
void BIT1_P(GB_CPU* cpu) { DEFERHL(BIT1); }
void SET1_A(GB_CPU* cpu) { SET1(cpu->A); }
void SET1_B(GB_CPU* cpu) { SET1(cpu->B); }
void SET1_C(GB_CPU* cpu) { SET1(cpu->C); }
void SET1_D(GB_CPU* cpu) { SET1(cpu->D); }
void SET1_E(GB_CPU* cpu) { SET1(cpu->E); }
void SET1_H(GB_CPU* cpu) { SET1(cpu->H); }
void SET1_L(GB_CPU* cpu) { SET1(cpu->L); }
void SET1_P(GB_CPU* cpu) { DEFERHL(SET1); }
void RES1_A(GB_CPU* cpu) { RES1(cpu->A); }
void RES1_B(GB_CPU* cpu) { RES1(cpu->B); }
void RES1_C(GB_CPU* cpu) { RES1(cpu->C); }
void RES1_D(GB_CPU* cpu) { RES1(cpu->D); }
void RES1_E(GB_CPU* cpu) { RES1(cpu->E); }
void RES1_H(GB_CPU* cpu) { RES1(cpu->H); }
void RES1_L(GB_CPU* cpu) { RES1(cpu->L); }
void RES1_P(GB_CPU* cpu) { DEFERHL(RES1); }

void BIT2_A(GB_CPU* cpu) { BIT2(cpu->A); }
void BIT2_B(GB_CPU* cpu) { BIT2(cpu->B); }
void BIT2_C(GB_CPU* cpu) { BIT2(cpu->C); }
void BIT2_D(GB_CPU* cpu) { BIT2(cpu->D); }
void BIT2_E(GB_CPU* cpu) { BIT2(cpu->E); }
void BIT2_H(GB_CPU* cpu) { BIT2(cpu->H); }
void BIT2_L(GB_CPU* cpu) { BIT2(cpu->L); }
void BIT2_P(GB_CPU* cpu) { DEFERHL(BIT2); }
void SET2_A(GB_CPU* cpu) { SET2(cpu->A); }
void SET2_B(GB_CPU* cpu) { SET2(cpu->B); }
void SET2_C(GB_CPU* cpu) { SET2(cpu->C); }
void SET2_D(GB_CPU* cpu) { SET2(cpu->D); }
void SET2_E(GB_CPU* cpu) { SET2(cpu->E); }
void SET2_H(GB_CPU* cpu) { SET2(cpu->H); }
void SET2_L(GB_CPU* cpu) { SET2(cpu->L); }
void SET2_P(GB_CPU* cpu) { DEFERHL(SET2); }
void RES2_A(GB_CPU* cpu) { RES2(cpu->A); }
void RES2_B(GB_CPU* cpu) { RES2(cpu->B); }
void RES2_C(GB_CPU* cpu) { RES2(cpu->C); }
void RES2_D(GB_CPU* cpu) { RES2(cpu->D); }
void RES2_E(GB_CPU* cpu) { RES2(cpu->E); }
void RES2_H(GB_CPU* cpu) { RES2(cpu->H); }
void RES2_L(GB_CPU* cpu) { RES2(cpu->L); }
void RES2_P(GB_CPU* cpu) { DEFERHL(RES2); }

void BIT3_A(GB_CPU* cpu) { BIT3(cpu->A); }
void BIT3_B(GB_CPU* cpu) { BIT3(cpu->B); }
void BIT3_C(GB_CPU* cpu) { BIT3(cpu->C); }
void BIT3_D(GB_CPU* cpu) { BIT3(cpu->D); }
void BIT3_E(GB_CPU* cpu) { BIT3(cpu->E); }
void BIT3_H(GB_CPU* cpu) { BIT3(cpu->H); }
void BIT3_L(GB_CPU* cpu) { BIT3(cpu->L); }
void BIT3_P(GB_CPU* cpu) { DEFERHL(BIT3); }
void SET3_A(GB_CPU* cpu) { SET3(cpu->A); }
void SET3_B(GB_CPU* cpu) { SET3(cpu->B); }
void SET3_C(GB_CPU* cpu) { SET3(cpu->C); }
void SET3_D(GB_CPU* cpu) { SET3(cpu->D); }
void SET3_E(GB_CPU* cpu) { SET3(cpu->E); }
void SET3_H(GB_CPU* cpu) { SET3(cpu->H); }
void SET3_L(GB_CPU* cpu) { SET3(cpu->L); }
void SET3_P(GB_CPU* cpu) { DEFERHL(SET3); }
void RES3_A(GB_CPU* cpu) { RES3(cpu->A); }
void RES3_B(GB_CPU* cpu) { RES3(cpu->B); }
void RES3_C(GB_CPU* cpu) { RES3(cpu->C); }
void RES3_D(GB_CPU* cpu) { RES3(cpu->D); }
void RES3_E(GB_CPU* cpu) { RES3(cpu->E); }
void RES3_H(GB_CPU* cpu) { RES3(cpu->H); }
void RES3_L(GB_CPU* cpu) { RES3(cpu->L); }
void RES3_P(GB_CPU* cpu) { DEFERHL(RES3); }

void BIT4_A(GB_CPU* cpu) { BIT4(cpu->A); }
void BIT4_B(GB_CPU* cpu) { BIT4(cpu->B); }
void BIT4_C(GB_CPU* cpu) { BIT4(cpu->C); }
void BIT4_D(GB_CPU* cpu) { BIT4(cpu->D); }
void BIT4_E(GB_CPU* cpu) { BIT4(cpu->E); }
void BIT4_H(GB_CPU* cpu) { BIT4(cpu->H); }
void BIT4_L(GB_CPU* cpu) { BIT4(cpu->L); }
void BIT4_P(GB_CPU* cpu) { DEFERHL(BIT4); }
void SET4_A(GB_CPU* cpu) { SET4(cpu->A); }
void SET4_B(GB_CPU* cpu) { SET4(cpu->B); }
void SET4_C(GB_CPU* cpu) { SET4(cpu->C); }
void SET4_D(GB_CPU* cpu) { SET4(cpu->D); }
void SET4_E(GB_CPU* cpu) { SET4(cpu->E); }
void SET4_H(GB_CPU* cpu) { SET4(cpu->H); }
void SET4_L(GB_CPU* cpu) { SET4(cpu->L); }
void SET4_P(GB_CPU* cpu) { DEFERHL(SET4); }
void RES4_A(GB_CPU* cpu) { RES4(cpu->A); }
void RES4_B(GB_CPU* cpu) { RES4(cpu->B); }
void RES4_C(GB_CPU* cpu) { RES4(cpu->C); }
void RES4_D(GB_CPU* cpu) { RES4(cpu->D); }
void RES4_E(GB_CPU* cpu) { RES4(cpu->E); }
void RES4_H(GB_CPU* cpu) { RES4(cpu->H); }
void RES4_L(GB_CPU* cpu) { RES4(cpu->L); }
void RES4_P(GB_CPU* cpu) { DEFERHL(RES4); }

void BIT5_A(GB_CPU* cpu) { BIT5(cpu->A); }
void BIT5_B(GB_CPU* cpu) { BIT5(cpu->B); }
void BIT5_C(GB_CPU* cpu) { BIT5(cpu->C); }
void BIT5_D(GB_CPU* cpu) { BIT5(cpu->D); }
void BIT5_E(GB_CPU* cpu) { BIT5(cpu->E); }
void BIT5_H(GB_CPU* cpu) { BIT5(cpu->H); }
void BIT5_L(GB_CPU* cpu) { BIT5(cpu->L); }
void BIT5_P(GB_CPU* cpu) { DEFERHL(BIT5); }
void SET5_A(GB_CPU* cpu) { SET5(cpu->A); }
void SET5_B(GB_CPU* cpu) { SET5(cpu->B); }
void SET5_C(GB_CPU* cpu) { SET5(cpu->C); }
void SET5_D(GB_CPU* cpu) { SET5(cpu->D); }
void SET5_E(GB_CPU* cpu) { SET5(cpu->E); }
void SET5_H(GB_CPU* cpu) { SET5(cpu->H); }
void SET5_L(GB_CPU* cpu) { SET5(cpu->L); }
void SET5_P(GB_CPU* cpu) { DEFERHL(SET5); }
void RES5_A(GB_CPU* cpu) { RES5(cpu->A); }
void RES5_B(GB_CPU* cpu) { RES5(cpu->B); }
void RES5_C(GB_CPU* cpu) { RES5(cpu->C); }
void RES5_D(GB_CPU* cpu) { RES5(cpu->D); }
void RES5_E(GB_CPU* cpu) { RES5(cpu->E); }
void RES5_H(GB_CPU* cpu) { RES5(cpu->H); }
void RES5_L(GB_CPU* cpu) { RES5(cpu->L); }
void RES5_P(GB_CPU* cpu) { DEFERHL(RES5); }

void BIT6_A(GB_CPU* cpu) { BIT6(cpu->A); }
void BIT6_B(GB_CPU* cpu) { BIT6(cpu->B); }
void BIT6_C(GB_CPU* cpu) { BIT6(cpu->C); }
void BIT6_D(GB_CPU* cpu) { BIT6(cpu->D); }
void BIT6_E(GB_CPU* cpu) { BIT6(cpu->E); }
void BIT6_H(GB_CPU* cpu) { BIT6(cpu->H); }
void BIT6_L(GB_CPU* cpu) { BIT6(cpu->L); }
void BIT6_P(GB_CPU* cpu) { DEFERHL(BIT6); }
void SET6_A(GB_CPU* cpu) { SET6(cpu->A); }
void SET6_B(GB_CPU* cpu) { SET6(cpu->B); }
void SET6_C(GB_CPU* cpu) { SET6(cpu->C); }
void SET6_D(GB_CPU* cpu) { SET6(cpu->D); }
void SET6_E(GB_CPU* cpu) { SET6(cpu->E); }
void SET6_H(GB_CPU* cpu) { SET6(cpu->H); }
void SET6_L(GB_CPU* cpu) { SET6(cpu->L); }
void SET6_P(GB_CPU* cpu) { DEFERHL(SET6); }
void RES6_A(GB_CPU* cpu) { RES6(cpu->A); }
void RES6_B(GB_CPU* cpu) { RES6(cpu->B); }
void RES6_C(GB_CPU* cpu) { RES6(cpu->C); }
void RES6_D(GB_CPU* cpu) { RES6(cpu->D); }
void RES6_E(GB_CPU* cpu) { RES6(cpu->E); }
void RES6_H(GB_CPU* cpu) { RES6(cpu->H); }
void RES6_L(GB_CPU* cpu) { RES6(cpu->L); }
void RES6_P(GB_CPU* cpu) { DEFERHL(RES6); }

void BIT7_A(GB_CPU* cpu) { BIT7(cpu->A); }
void BIT7_B(GB_CPU* cpu) { BIT7(cpu->B); }
void BIT7_C(GB_CPU* cpu) { BIT7(cpu->C); }
void BIT7_D(GB_CPU* cpu) { BIT7(cpu->D); }
void BIT7_E(GB_CPU* cpu) { BIT7(cpu->E); }
void BIT7_H(GB_CPU* cpu) { BIT7(cpu->H); }
void BIT7_L(GB_CPU* cpu) { BIT7(cpu->L); }
void BIT7_P(GB_CPU* cpu) { DEFERHL(BIT7); }
void SET7_A(GB_CPU* cpu) { SET7(cpu->A); }
void SET7_B(GB_CPU* cpu) { SET7(cpu->B); }
void SET7_C(GB_CPU* cpu) { SET7(cpu->C); }
void SET7_D(GB_CPU* cpu) { SET7(cpu->D); }
void SET7_E(GB_CPU* cpu) { SET7(cpu->E); }
void SET7_H(GB_CPU* cpu) { SET7(cpu->H); }
void SET7_L(GB_CPU* cpu) { SET7(cpu->L); }
void SET7_P(GB_CPU* cpu) { DEFERHL(SET7); }
void RES7_A(GB_CPU* cpu) { RES7(cpu->A); }
void RES7_B(GB_CPU* cpu) { RES7(cpu->B); }
void RES7_C(GB_CPU* cpu) { RES7(cpu->C); }
void RES7_D(GB_CPU* cpu) { RES7(cpu->D); }
void RES7_E(GB_CPU* cpu) { RES7(cpu->E); }
void RES7_H(GB_CPU* cpu) { RES7(cpu->H); }
void RES7_L(GB_CPU* cpu) { RES7(cpu->L); }
void RES7_P(GB_CPU* cpu) { DEFERHL(RES7); }

/* JUMPS */
void JP(GB_CPU* cpu) {
    GB_WORD addr16 = readWord(cpu->parent->mem, cpu->PC);
    cpu->PC = addr16;
    cpu->insnCycles = 4; // fix time
}

void JPMISS(GB_CPU* cpu) {
	cpu->PC += 2;
}

void JPNZ(GB_CPU* cpu) { if (!FLAG_ZERO(cpu)) JP(cpu); else JPMISS(cpu); }
void JPZO(GB_CPU* cpu) { if (FLAG_ZERO(cpu)) JP(cpu); else JPMISS(cpu); }
void JPNC(GB_CPU* cpu) { if (!FLAG_CARR(cpu)) JP(cpu); else JPMISS(cpu); }
void JPCA(GB_CPU* cpu) { if (FLAG_CARR(cpu)) JP(cpu); else JPMISS(cpu); }

void JPHL(GB_CPU* cpu) { cpu->PC = cpu->HL;	}

void JR(GB_CPU* cpu) {
	GB_SBYTE offset = readByte(cpu->parent->mem, cpu->PC++);
	cpu->PC += offset;
	cpu->insnCycles = 3; // fix time
}

void JRMISS(GB_CPU* cpu) {
	cpu->PC++;
}

void JRNZ(GB_CPU* cpu) { if (!FLAG_ZERO(cpu)) JR(cpu); else JRMISS(cpu); }
void JRZO(GB_CPU* cpu) { if (FLAG_ZERO(cpu)) JR(cpu); else JRMISS(cpu); }
void JRNC(GB_CPU* cpu) { if (!FLAG_CARR(cpu)) JR(cpu); else JRMISS(cpu); }
void JRCA(GB_CPU* cpu) { if (FLAG_CARR(cpu)) JR(cpu); else JRMISS(cpu); }

/* CALL & RETURN */
void CALL(GB_CPU* cpu) {
	GB_WORD addr16 = readWord(cpu->parent->mem, cpu->PC); cpu->PC += 2;
	PUSH(cpu->PC);
	cpu->PC = addr16;
	cpu->insnCycles = 6; // fix time
}

void CALLMISS(GB_CPU* cpu) {
	cpu->PC += 2;
}

void CALLNZ(GB_CPU* cpu) { if (!FLAG_ZERO(cpu)) CALL(cpu); else CALLMISS(cpu); }
void CALLZO(GB_CPU* cpu) { if (FLAG_ZERO(cpu)) CALL(cpu); else CALLMISS(cpu); }
void CALLNC(GB_CPU* cpu) { if (!FLAG_CARR(cpu)) CALL(cpu); else CALLMISS(cpu); }
void CALLCA(GB_CPU* cpu) { if (FLAG_CARR(cpu)) CALL(cpu); else CALLMISS(cpu); }

void RET(GB_CPU* cpu) {
	POP(cpu->PC);
	cpu->insnCycles = 5; // fix time
}

void RETMISS(GB_CPU* cpu) { }

void RETNZ(GB_CPU* cpu) { if (!FLAG_ZERO(cpu)) RET(cpu); else RETMISS(cpu); }
void RETZO(GB_CPU* cpu) { if (FLAG_ZERO(cpu)) RET(cpu); else RETMISS(cpu); }
void RETNC(GB_CPU* cpu) { if (!FLAG_CARR(cpu)) RET(cpu); else RETMISS(cpu); }
void RETCA(GB_CPU* cpu) { if (FLAG_CARR(cpu)) RET(cpu); else RETMISS(cpu); }

void RETI(GB_CPU* cpu) { EI(cpu);  RET(cpu); }

/* RESTART */
void RST00(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x00; }
void RST08(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x08; }
void RST10(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x10; }
void RST18(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x18; }
void RST20(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x20; }
void RST28(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x28; }
void RST30(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x30; }
void RST38(GB_CPU* cpu) { PUSH(cpu->PC);  cpu->PC = 0x38; }

/* INTERRUPTS */
void INT(GB_CPU* cpu, GB_WORD addr) {
	DI(cpu);
	PUSH(cpu->PC);
	cpu->PC = addr;
	cpu->insnCycles = 5;
}
