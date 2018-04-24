#include <sys/emu/emu16_64.hpp>
#include <sys/emu/emu_alu.hpp>
#include <sys/emu/emu_mm.hpp>
#include <sys/emu/emu_instruc.hpp>

using namespace emu;
using namespace emu::alu;
using namespace emu::instructions;

Emulator16::Emulator16() {}

uint8_t  Emulator16::peekb(uint16_t segment, uint16_t offset) { return mm::peekb(segment, offset); }
uint16_t Emulator16::peekw(uint16_t segment, uint16_t offset) { return mm::peekw(segment, offset); }
uint32_t Emulator16::peekd(uint16_t segment, uint16_t offset) { return mm::peekd(segment, offset); }

void Emulator16::pokeb(uint16_t segment, uint16_t offset, uint8_t  value) { mm::pokeb(segment, offset, value); }
void Emulator16::pokew(uint16_t segment, uint16_t offset, uint16_t value) { mm::pokew(segment, offset, value); }
void Emulator16::poked(uint16_t segment, uint16_t offset, uint32_t value) { mm::poked(segment, offset, value); }

#define SHORT false
#define WIDE true

#define H73_IS_REPE true
#define H73_IS_REP false
#define NOPREFIX false

#define ALUOP { "ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP" }

#define REGNAME8_0 "AL"
#define REGNAME8_1 "CL"
#define REGNAME8_2 "DL"
#define REGNAME8_3 "BL"
#define REGNAME8_4 "AH"
#define REGNAME8_5 "CH"
#define REGNAME8_6 "DH"
#define REGNAME8_7 "BH"
#define REGNAME8(regn) REGNAME8_ ## regn

#define REGNAME16_32_0 "xAX"
#define REGNAME16_32_1 "xCX"
#define REGNAME16_32_2 "xDX"
#define REGNAME16_32_3 "xBX"
#define REGNAME16_32_4 "xSP"
#define REGNAME16_32_5 "xBP"
#define REGNAME16_32_6 "xSI"
#define REGNAME16_32_7 "xDI"
#define REGNAME16_32(regn) REGNAME16_32_ ## regn

#define NO_ARG_SELECTOR nullptr
#define NO_VAL_SELECTOR nullptr
#define NO_ACTION nullptr

SELECTOR_SEGMENT_REG(op_es, ES)
SELECTOR_SEGMENT_REG(op_ds, DS)

static const char *op_mov_r_i_nameSelector(cpu_instruction_t &instruction) {
    return registerName(instruction.opcode & 7, !(instruction.opcode & 8), instruction.operandSizePrefix);
}

static cpu_op_argument_value_t op_mov_r_i_valSelector(Emulator16 *emu, cpu_instruction_t &instruction) {
    unsigned size = instruction.opcode & 8 ? (instruction.operandSizePrefix ? sizeof(uint32_t) : sizeof(uint16_t)) : sizeof(uint8_t);

    return {true, false, reinterpret_cast<uintptr_t>(emu->regPtrByIx(instruction.opcode & 7)), 0, size};
}

static const char *op_mov_sreg_rm16_nameSelector(cpu_instruction_t &instruction) {
    return segRegisterName(instruction.modrm.regOrOpcode);
}

static cpu_op_argument_value_t op_mov_sreg_rm16_valSelector(Emulator16 *emu, cpu_instruction_t &instruction) {
    return {true, false, reinterpret_cast<uintptr_t>(emu->regPtrByIx(instruction.modrm.regOrOpcode)), 0, sizeof(uint16_t)};
}

#define MOV_OPCODE_I8(regn) \
    { 0xB0 + regn, 0, SHORT, NOPREFIX, DS_INDEX, UNIQUE_RMS("MOV"), \
        op_mov_r_i_nameSelector, op_mov_r_i_valSelector, CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_IMM8, NO_ACTION }

#define MOV_OPCODE_I16_32(regn) \
    { 0xB8 + regn, 0, SHORT, NOPREFIX, DS_INDEX, UNIQUE_RMS("MOV"), \
        op_mov_r_i_nameSelector, op_mov_r_i_valSelector, CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_IMM16_32, NO_ACTION }

void op_generic_alu_action(Emulator16 *emu, cpu_instruction_t &instruction, cpu_op_argument_value_t lhs, cpu_op_argument_value_t rhs) {
    switch (lhs.argumentSize) {
        case 1: emu->eflags = sub<uint8_t>(lhs.val(), rhs.val(), static_cast<alu_flags_t>(emu->eflags)).newFlags; return;
        case 2: emu->eflags = sub<uint16_t>(lhs.val(), rhs.val(), static_cast<alu_flags_t>(emu->eflags)).newFlags; return;
        case 4: emu->eflags = sub<uint32_t>(lhs.val(), rhs.val(), static_cast<alu_flags_t>(emu->eflags)).newFlags; return;
    }
    debug::printf("invalid lhs arg size: %d", lhs.argumentSize);
    panic();
}

cpu_opcode_t opcodes[] = {
    { 0x06, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("PUSH"), op_es_nameSelector, op_es_valSelector,
        CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x07, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("POP"), op_es_nameSelector, op_es_valSelector,
        CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x1E, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("PUSH"), op_ds_nameSelector, op_ds_valSelector,
        CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x1f, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("POP"), op_ds_nameSelector, op_ds_valSelector,
        CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x3C, 0, SHORT, NOPREFIX, DS_INDEX, UNIQUE_RMS("CMP"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_AL, CPU_OP_ARGUMENT_IMM8, NO_ACTION },

    { 0x3D, 0, SHORT, NOPREFIX, DS_INDEX, UNIQUE_RMS("CMP"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_VAX, CPU_OP_ARGUMENT_IMM16_32, NO_ACTION },

    { 0x60, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("PUSHA"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x61, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("POPA"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x74, 0, SHORT, NOPREFIX, CS_INDEX, UNIQUE_RMS("JZ"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_REL8, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0x75, 0, SHORT, NOPREFIX, CS_INDEX, UNIQUE_RMS("JNZ"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_REL8, CPU_OP_ARGUMENT_NONE, [](auto emu, auto instruction, auto lhs, auto rhs) -> auto {
            if (!(emu->eflags & ALU_FLAG_ZERO)) emu->eip = lhs.val();
        } },

    { 0x80, 0, SHORT, NOPREFIX, DS_INDEX, ALUOP, NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_RM8, CPU_OP_ARGUMENT_IMM8, op_generic_alu_action },

    { 0x8E, 0, SHORT, NOPREFIX, DS_INDEX, UNIQUE_RMS("MOV"), op_mov_sreg_rm16_nameSelector, op_mov_sreg_rm16_valSelector,
        CPU_OP_ARGUMENT_IMPLICIT, CPU_OP_ARGUMENT_RM16_32, NO_ACTION }, /* TODO: only 16bit arg */

    { 0x9c, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("PUSHF"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, [](auto emu, auto instruction, auto lhs, auto rhs) -> auto { emu->pushWordOnStack(emu->eflags); } },

    { 0x9d, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("POPF"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    MOV_OPCODE_I8(0), MOV_OPCODE_I8(1), MOV_OPCODE_I8(2), MOV_OPCODE_I8(3),
    MOV_OPCODE_I8(4), MOV_OPCODE_I8(5), MOV_OPCODE_I8(6), MOV_OPCODE_I8(7),

    MOV_OPCODE_I16_32(0), MOV_OPCODE_I16_32(1), MOV_OPCODE_I16_32(2), MOV_OPCODE_I16_32(3),
    MOV_OPCODE_I16_32(4), MOV_OPCODE_I16_32(5), MOV_OPCODE_I16_32(6), MOV_OPCODE_I16_32(7),

    { 0xcf, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("IRET"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, NO_ACTION },

    { 0xe8, 0, SHORT, NOPREFIX, CS_INDEX, UNIQUE_RMS("CALL"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_REL16_32, CPU_OP_ARGUMENT_NONE, NO_ACTION},
        
    { 0xe9, 0, SHORT, NOPREFIX, CS_INDEX, UNIQUE_RMS("JMP"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_REL16_32, CPU_OP_ARGUMENT_NONE, NO_ACTION },
        
    { 0xeb, 0, SHORT, NOPREFIX, CS_INDEX, UNIQUE_RMS("JMP"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_REL8, CPU_OP_ARGUMENT_NONE, NO_ACTION },
        
    { 0xfa, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("CLI"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, [](auto emu, auto instruction, auto lhs, auto rhs) -> auto {} },
        
    /* 0xfb, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("STI"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, NO_ACTION },*/
        
    { 0xfc, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("CLD"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, [](auto emu, auto instruction, auto lhs, auto rhs) -> auto {} },
        
    /*{ 0xfd, 0, SHORT, NOPREFIX, NOT_INDEXED, UNIQUE_RMS("STD"), NO_ARG_SELECTOR, NO_VAL_SELECTOR,
        CPU_OP_ARGUMENT_NONE, CPU_OP_ARGUMENT_NONE, NO_ACTION }*/
};

void Emulator16::step() {
    auto instr = fetch(cs, eip, sizeof(opcodes) / sizeof(cpu_opcode_t), opcodes);
    if (instr.opcodeDef.action == nullptr) panic_msg("opcodeDef.action == nullptr");
    eip = instr.nextEip;
    instr.opcodeDef.action(this, instr, decodeArg(this, instr, false), decodeArg(this, instr, true));
    debug::printf("eip: %x\n", eip);
}