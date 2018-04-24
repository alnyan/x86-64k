#include <sys/emu/emu_instruc.hpp>
#include <sys/emu/emu_mm.hpp>
#include <sys/debug.hpp>
#include <sys/panic.hpp>

using namespace emu::instructions;
using namespace emu::mm;
using namespace debug;

#define ASSERT_EIP_OVERFLOW(eip) { if (reinterpret_cast<uintptr_t>(eip) > 0xffff) panic_msg("eip overflow"); }

const char *emu::instructions::registerName(int ix, bool byte, bool opSize) {
    switch (ix) {
        case 0: return byte ? "AL" : opSize ? "EAX" : "AX";
        case 1: return byte ? "CL" : opSize ? "ECX" : "CX";
        case 2: return byte ? "DL" : opSize ? "EDX" : "DX";
        case 3: return byte ? "BL" : opSize ? "EBX" : "BX";
        case 4: return byte ? "AH" : opSize ? "ESP" : "SP";
        case 5: return byte ? "CH" : opSize ? "EBP" : "BP";
        case 6: return byte ? "DH" : opSize ? "ESI" : "SI";
        case 7: return byte ? "BH" : opSize ? "EDI" : "DI";
    }

    panic_msg("invalid register index");
}

const char *emu::instructions::segRegisterName(int ix) {
    switch (ix) {
        case 0: return "ES";
        case 1: return "CS";
        case 2: return "SS";
        case 3: return "DS";
        case 4: return "FS";
        case 5: return "GS";
    }

    panic_msg("invalid register index");
}

void print_modrm_pointer(cpu_instruction_t &instruction, bool byte) {
    if (instruction.addressSizePrefix) panic_msg("address-size prefix is not supported");
    const char *sizePrefix = byte ? "byte ptr" : instruction.operandSizePrefix ? "dword ptr" : "word ptr";
    switch (instruction.modrm.mod) {
        case 0: 
            switch (instruction.modrm.rm) {
                case 0: printf("%s [BX + SI]", sizePrefix); break;
                case 1: printf("%s [BX + DI]", sizePrefix); break;
                case 2: printf("%s [BP + SI]", sizePrefix); break;
                case 3: printf("%s [BP + DI]", sizePrefix); break;
                case 4: printf("%s [SI]", sizePrefix); break;
                case 5: printf("%s [SI]", sizePrefix); break;
                case 6: printf("[0x%04x]", sizePrefix, instruction.displacement); break;
                case 7: printf("%s [BX]", sizePrefix); break;
            }
        case 1:
            switch (instruction.modrm.rm) {
                case 0: printf("%s [BX + SI + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 1: printf("%s [BX + DI + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 2: printf("%s [BP + SI + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 3: printf("%s [BP + DI + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 4: printf("%s [SI + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 5: printf("%s [SI + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 6: printf("%s [BP + 0x%02x]", sizePrefix, instruction.displacement); break;
                case 7: printf("%s [BX + 0x%02x]", sizePrefix, instruction.displacement); break;
            }
        case 2:
            switch (instruction.modrm.rm) {
                case 0: printf("%s [BX + SI + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 1: printf("%s [BX + DI + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 2: printf("%s [BP + SI + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 3: printf("%s [BP + DI + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 4: printf("%s [SI + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 5: printf("%s [SI + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 6: printf("%s [BP + 0x%04x]", sizePrefix, instruction.displacement); break;
                case 7: printf("%s [BX + 0x%04x]", sizePrefix, instruction.displacement); break;
            }
        case 3:
            print(registerName(instruction.modrm.rm, byte, instruction.operandSizePrefix));
            break;
    }
}

void print_modrm_register(cpu_instruction_t &instruction, bool byte) {
    print(registerName(instruction.modrm.regOrOpcode, byte, instruction.operandSizePrefix));
}

#define TRUNCATE_16BIT_PTR(ptr) ((ptr) & 0xffff)

void print_instruction(cpu_instruction_t &instruction, void *startEip) {
    printf("%08x ", startEip);
    switch (instruction.instructionPrefix) {
        case CPU_INSTR_NONE:  break;
        case CPU_INSTR_REP:   print(instruction.opcodeDef.treatRepAsRepe ? "REPE " : "REP "); break;
        case CPU_INSTR_REPNE: print("REPNE "); break;
        case CPU_INSTR_LOCK:  print("LOCK "); break;
    }
    
    printf("%s ", instruction.validName());
    
    switch (instruction.opcodeDef.lhs) {
        case CPU_OP_ARGUMENT_IMPLICIT:
            if (instruction.opcodeDef.implicitSelector == nullptr) panic_msg("implicit selector is nullptr");
            print(instruction.opcodeDef.implicitSelector(instruction));
            break;
        case CPU_OP_ARGUMENT_NONE:
            break;
        case CPU_OP_ARGUMENT_RM8:
        case CPU_OP_ARGUMENT_M8:
            print_modrm_pointer(instruction, true);
            break;
        case CPU_OP_ARGUMENT_RM16_32:
        case CPU_OP_ARGUMENT_M16_32:
            print_modrm_pointer(instruction, false);
            break;
        case CPU_OP_ARGUMENT_IMM8:
        case CPU_OP_ARGUMENT_IMM16_32:
            printf("0x%x", instruction.immediate);
            break;
        case CPU_OP_ARGUMENT_REL8:
            printf("0x%04x", TRUNCATE_16BIT_PTR(instruction.nextEip + static_cast<int8_t>(static_cast<uint8_t>(instruction.immediate))));
            break;
        case CPU_OP_ARGUMENT_REL16_32:
            if (instruction.operandSizePrefix)
                printf("0x%04x", TRUNCATE_16BIT_PTR(instruction.nextEip + static_cast<int32_t>(static_cast<uint32_t>(instruction.immediate))));
            else
                printf("0x%04x", TRUNCATE_16BIT_PTR(instruction.nextEip + static_cast<int16_t>(static_cast<uint16_t>(instruction.immediate))));
            break;
        case CPU_OP_ARGUMENT_R8:
            print_modrm_register(instruction, true);
            break;
        case CPU_OP_ARGUMENT_R16_32:
            print_modrm_register(instruction, false);
            break;
        case CPU_OP_ARGUMENT_AL:
            print("AL");
            break;
        case CPU_OP_ARGUMENT_AX:
            print("AX");
            break;
        case CPU_OP_ARGUMENT_VAX:
            print(instruction.operandSizePrefix ? "EAX" : "AX");
            break;
        case CPU_OP_ARGUMENT_M16:
        default:
            panic_msg("unsupported op argument (lhs)");
    }

    if (instruction.opcodeDef.lhs == CPU_OP_ARGUMENT_NONE && instruction.opcodeDef.rhs != CPU_OP_ARGUMENT_NONE)
        panic_msg("lhs is NONE when rhs is not NONE");

    if (instruction.opcodeDef.rhs != CPU_OP_ARGUMENT_NONE) print(", ");

    switch (instruction.opcodeDef.rhs) {
        case CPU_OP_ARGUMENT_NONE:
            break;
        case CPU_OP_ARGUMENT_RM8:
        case CPU_OP_ARGUMENT_M8:
            print_modrm_pointer(instruction, true);
            break;
        case CPU_OP_ARGUMENT_RM16_32:
        case CPU_OP_ARGUMENT_M16_32:
            print_modrm_pointer(instruction, false);
            break;
        case CPU_OP_ARGUMENT_IMM8:
        case CPU_OP_ARGUMENT_IMM16_32:
            printf("0x%x", instruction.immediate);
            break;
        case CPU_OP_ARGUMENT_REL8:
            printf("0x%04x", TRUNCATE_16BIT_PTR(instruction.nextEip + static_cast<int8_t>(static_cast<uint8_t>(instruction.immediate))));
            break;
        case CPU_OP_ARGUMENT_REL16_32:
            if (instruction.operandSizePrefix)
                printf("0x%04x", TRUNCATE_16BIT_PTR(instruction.nextEip + static_cast<int32_t>(static_cast<uint32_t>(instruction.immediate))));
            else
                printf("0x%04x", TRUNCATE_16BIT_PTR(instruction.nextEip + static_cast<int16_t>(static_cast<uint16_t>(instruction.immediate))));
            break;
        case CPU_OP_ARGUMENT_R8:
            print_modrm_register(instruction, true);
            break;
        case CPU_OP_ARGUMENT_R16_32:
            print_modrm_register(instruction, false);
            break;
        case CPU_OP_ARGUMENT_AL:
            print("AL");
            break;
        case CPU_OP_ARGUMENT_AX:
            print("AX");
            break;
        case CPU_OP_ARGUMENT_VAX:
            print(instruction.operandSizePrefix ? "EAX" : "AX");
            break;
        case CPU_OP_ARGUMENT_M16:
        default:
            panic_msg("unsupported op argument (lhs)");
    }

    printf("\n");
}

cpu_instruction_t emu::instructions::fetch(uint16_t cs, uint32_t inEip, int opcodeCount, const cpu_opcode_t *opcodes) {
    uint8_t *eip = reinterpret_cast<uint8_t*>(addrToLinear(cs, inEip));
    uint8_t *startEip = eip;
    cpu_instruction_t instruction = {};
    
    switch (*eip) {
        case CPU_INSTR_LOCK:
        case CPU_INSTR_REP:
        case CPU_INSTR_REPNE:
            instruction.instructionPrefix = static_cast<cpu_instruction_prefix_t>(*eip++);
            break;
    }

    if (*eip == CPU_ADDRESS_SIZE_OVERRIDE) {
        panic_msg("CPU_ADDRESS_SIZE_OVERRIDE does not supported");
        instruction.addressSizePrefix = true;
        eip++;
    }

    if (*eip == CPU_OPERAND_SIZE_OVERRIDE) {
        panic_msg("CPU_OPERAND_SIZE_OVERRIDE does not supported");
        instruction.operandSizePrefix = true;
        eip++;
    }

    switch (*eip) {
        case CPU_SEG_CS:
        case CPU_SEG_DS:
        case CPU_SEG_ES:
        case CPU_SEG_SS:
        case CPU_SEG_FS:
        case CPU_SEG_GS:
            instruction.segmentOverridePrefix = static_cast<cpu_segment_override_t>(*eip++);
            break;
    }

    uint8_t higherOpcode = *eip++;

    for (int i = 0; i < opcodeCount; i++) {
        if (opcodes[i].higherByte == higherOpcode) {
            if (opcodes[i].isWide) {
                uint8_t lowerOp = *eip++;
                if (opcodes[i].lowerByte != lowerOp) continue;
                instruction.opcode = (higherOpcode << 8) | lowerOp;
            }
            else {
                instruction.opcode = higherOpcode;
            }

            instruction.opcodeDef = opcodes[i];

            if (opcodes[i].requiresModrm()) {
                *reinterpret_cast<uint8_t*>(&instruction.modrm) = *eip++;

                switch (instruction.modrm.mod) { // TODO: 32-bit modrms
                    uint8_t higher, lower, disp;
                    case 0:
                        if (instruction.modrm.rm == 6) { // only r/m 110 have disp16
                            higher = *eip++;
                            lower = *eip++;
                            instruction.hasDisplacement = true;
                            instruction.displacement = (higher << 8) | lower;
                        }
                        break;
                    case 1: // any r/ms have disp8
                        disp = *eip++;
                        instruction.hasDisplacement = true;
                        instruction.displacement = disp;
                        break;
                    case 2: // any r/ms have disp16
                        higher = *eip++;
                        lower = *eip++;
                        instruction.hasDisplacement = true;
                        instruction.displacement = (higher << 8) | lower;
                        break;
                }
            }

            if (opcodes[i].requiresImmediate()) {
                switch (opcodes[i].immediateSize()) {
                    uint8_t b0, b1, b2, b3;
                    case CPU_IMM_NONE: 
                        printf("Opcode %s requires immediate value, but size is NONE", instruction.validName());
                        panic();
                        break;
                    case CPU_IMM_BYTE:
                        b0 = *eip++;
                        instruction.immediate = b0;
                        instruction.hasImmediate = true;
                        break;
                    case CPU_IMM_WORD_DWORD:
                        if (instruction.operandSizePrefix) {
                            b0 = *eip++;
                            b1 = *eip++;
                            b2 = *eip++;
                            b3 = *eip++;
                            instruction.immediate = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
                        }
                        else {
                            b0 = *eip++;
                            b1 = *eip++;
                            instruction.immediate = (b1 << 8) | b0;
                        }
                        instruction.hasImmediate = true;
                        break;
                }
            }
            instruction.nextEip = extractOffset(cs, reinterpret_cast<uintptr_t>(eip));
            print_instruction(instruction, startEip);
            return instruction;
        }
    }
    
    printf("Unknown opcode: %02x\n", higherOpcode);
    panic();
}