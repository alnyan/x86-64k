#pragma once
#include <stdint.h>
#include <sys/emu/emu16_64.hpp>

using namespace emu;

#define CS cs
#define DS ds
#define ES es
#define SS ss
#define FS fs
#define GS gs

#define UNIQUE_RMS(name) { (name), (name), (name), (name), (name), (name), (name), (name) }
#define SELECTOR_SEGMENT_REG(selPrefix, seg) static const char *selPrefix##_nameSelector(cpu_instruction_t &instr) { return #seg; } \
static cpu_op_argument_value_t selPrefix##_valSelector(Emulator16 *emu, cpu_instruction_t &instr) \
{ return { true, false, reinterpret_cast<uintptr_t>(&emu->seg), 0, sizeof(uint16_t)}; }

namespace emu {
    namespace instructions {
        struct modrm_t {
            uint8_t rm : 3;
            uint8_t regOrOpcode : 3;
            uint8_t mod : 2;
        };

        struct sib_t {
            uint8_t base : 3;
            uint8_t index : 3;
            uint8_t ss : 2;
        };

        enum cpu_instruction_prefix_t : uint8_t { 
            CPU_INSTR_NONE = 0,
            CPU_INSTR_REP = 0xf3,
            CPU_INSTR_REPNE = 0xf2,
            CPU_INSTR_LOCK = 0xf0
        };

        enum cpu_segment_override_t : uint8_t { 
            CPU_SEG_DEFAULT = 0, 
            CPU_SEG_CS = 0x2e, 
            CPU_SEG_SS = 0x36, 
            CPU_SEG_DS = 0x3e, 
            CPU_SEG_ES = 0x26, 
            CPU_SEG_FS = 0x64,
            CPU_SEG_GS = 0x65 
        };

#define REQUIRES_MODRM(arg) (\
(arg) == CPU_OP_ARGUMENT_RM8 ||\
(arg) == CPU_OP_ARGUMENT_RM16_32 ||\
(arg) == CPU_OP_ARGUMENT_R8 ||\
(arg) == CPU_OP_ARGUMENT_R16_32 ||\
(arg) == CPU_OP_ARGUMENT_M8 ||\
(arg) == CPU_OP_ARGUMENT_M16 ||\
(arg) == CPU_OP_ARGUMENT_M16_32)

#define REQUIRES_IMMEDIATE(arg) (\
(arg) == CPU_OP_ARGUMENT_IMM8 ||\
(arg) == CPU_OP_ARGUMENT_IMM16_32 ||\
(arg) == CPU_OP_ARGUMENT_REL8 ||\
(arg) == CPU_OP_ARGUMENT_REL16_32)

#define IMMEDIATE_SIZE(arg) (((arg) == CPU_OP_ARGUMENT_IMM8) || ((arg) == CPU_OP_ARGUMENT_REL8) ? CPU_IMM_BYTE : CPU_IMM_WORD_DWORD)

        enum cpu_op_argument_t {
            // no argument
            CPU_OP_ARGUMENT_NONE,
            CPU_OP_ARGUMENT_IMPLICIT,
            
            // r/m argument
            CPU_OP_ARGUMENT_RM8,
            CPU_OP_ARGUMENT_RM16_32,
            
            // register argument
            CPU_OP_ARGUMENT_R8,
            CPU_OP_ARGUMENT_R16_32,
            
            // immediate argument
            CPU_OP_ARGUMENT_IMM8,
            CPU_OP_ARGUMENT_IMM16_32,

            // relative immediate argument
            CPU_OP_ARGUMENT_REL8,
            CPU_OP_ARGUMENT_REL16_32,
            
            // memory argument
            CPU_OP_ARGUMENT_M8,
            CPU_OP_ARGUMENT_M16,
            CPU_OP_ARGUMENT_M16_32,
            
            // specialized register argument
            // rAX
            CPU_OP_ARGUMENT_AL,
            CPU_OP_ARGUMENT_AX,
            CPU_OP_ARGUMENT_VAX,
            
            // rDX
            CPU_OP_ARGUMENT_DX
        };

        const uint8_t CPU_OPERAND_SIZE_OVERRIDE = 0x66;
        const uint8_t CPU_ADDRESS_SIZE_OVERRIDE = 0x67;

        enum opcode_addr_size_t { CPU_ADDR_NONE, CPU_ADDR_BYTE, CPU_ADDR_WORD_DWORD };
        enum opcode_immediate_size_t { CPU_IMM_NONE, CPU_IMM_BYTE, CPU_IMM_WORD_DWORD };

        struct cpu_instruction_t;

        struct cpu_op_argument_value_t {
            bool exists;
            bool isImmediate;
            uintptr_t reference;
            uint32_t immediate;
            unsigned argumentSize;

            static uint32_t trunc(uint32_t val, unsigned size) {
                return val & ((1 << (size * 8 - 1)) * 2 + 1);
            }
            uint32_t val() { return isImmediate ? immediate : trunc(*reinterpret_cast<uint32_t*>(reference), argumentSize); }
        };

        typedef const char* (*cpu_op_implicit_arg_selector_t)(cpu_instruction_t &instruction); 
        typedef cpu_op_argument_value_t (*cpu_op_implicit_arg_val_selector_t)(Emulator16 *emu, cpu_instruction_t &instruction); 
        typedef void (*cpu_op_action_t)(Emulator16 *emu, cpu_instruction_t &instruction, cpu_op_argument_value_t lhs, cpu_op_argument_value_t rhs); 

        struct cpu_opcode_t {
            uint8_t higherByte;
            uint8_t lowerByte;
            bool isWide;
            bool treatRepAsRepe;
            int defaultSegIndex;
            const char *names[8]; // different names for different MOD/RMs
            cpu_op_implicit_arg_selector_t implicitNameSelector; // can be nullptr if lhs is explicit
            cpu_op_implicit_arg_val_selector_t implicitValSelector; // can be nullptr if lhs is explicit
            cpu_op_argument_t lhs, rhs;
            cpu_op_action_t action;

            bool requiresModrm() const { return REQUIRES_MODRM(lhs) || REQUIRES_MODRM(rhs); }
            bool requiresImmediate() const { return REQUIRES_IMMEDIATE(lhs) || REQUIRES_IMMEDIATE(rhs); }
            opcode_immediate_size_t immediateSize() const {
                return REQUIRES_IMMEDIATE(lhs) ? IMMEDIATE_SIZE(lhs) : REQUIRES_IMMEDIATE(rhs) ? IMMEDIATE_SIZE(rhs) : CPU_IMM_NONE;
            }
            // opcode_func will be added later
        };

        struct cpu_instruction_t {
            cpu_opcode_t opcodeDef;

            cpu_instruction_prefix_t instructionPrefix;
            bool addressSizePrefix;
            bool operandSizePrefix;
            cpu_segment_override_t segmentOverridePrefix;

            uint16_t opcode;
            modrm_t modrm;
            sib_t sib; // not used in x86-16

            bool hasDisplacement;
            uint32_t displacement;
            
            bool hasImmediate;
            uint32_t immediate;

            uint32_t nextEip;

            const char *validName() { return opcodeDef.names[modrm.regOrOpcode]; }
        };

        const char *registerName(int ix, bool byte, bool opSize);
        const char *segRegisterName(int ix);
        cpu_instruction_t fetch(uint16_t cs, uint32_t inEip, int opcodeCount, const cpu_opcode_t *opcodes);
        cpu_op_argument_value_t decodeArg(Emulator16 *emu, cpu_instruction_t &instruction, bool rhs);
    }
}