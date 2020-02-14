#pragma once
#include "GB_memory.h"

GB_BYTE readByteDefault(GB_MEMORY* mem, GB_WORD addr);

void writeByteDefault(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val);

GB_BYTE readByteMbc1(GB_MEMORY* mem, GB_WORD addr);

void writeByteMbc1(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val);

GB_BYTE readByteMbc2(GB_MEMORY* mem, GB_WORD addr);

void writeByteMbc2(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val);

GB_BYTE readByteMbc3(GB_MEMORY* mem, GB_WORD addr);

void writeByteMbc3(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val);

GB_BYTE readByteMbc5(GB_MEMORY* mem, GB_WORD addr);

void writeByteMbc5(GB_MEMORY* mem, GB_WORD addr, GB_BYTE val);