#include <dev/vesa.hpp>
#include <sys/debug.hpp>
#include <sys/panic.hpp>

using namespace debug;

struct ivt_entry {
    uint16_t offset;
    uint16_t segment;
};

#define VESA_VIDEO_INTERRUPT 0x10

#define MMAP_BIOS_ZONE_START 0xa0000
#define MMAP_BIOS_ZONE_END 0xfffff

#define HANDLER_PTR_LOOKS_LIKE_VALID(ptr) (((ptr) >= MMAP_BIOS_ZONE_START) && ((ptr) <= MMAP_BIOS_ZONE_END))
#define RETURN_SEGMENT_MAGIC 0xdead
#define RETURN_OFFSET_MAGIC 0xbabe

#define REALMODE_SEGMENT_MULTIPLIER 0x10
#define CPU_SEGMENT_BOUNDARY 0x10000
#define CPU_REAL_TO_LINEAR(seg, ofs) ((seg) * REALMODE_SEGMENT_MULTIPLIER + (ofs))
#define CPU_LINEAR_TO_OFFSET(seg, ofs) ((ofs) - ((seg) * REALMODE_SEGMENT_MULTIPLIER))

ivt_entry int10handler;

uint16_t cs, ss, es, ds, fs, gs;
uint32_t ax, bx, cx, dx, si, di, sp, bp, ip, flags;

static inline void eraseAllRegisters() {
    ax = bx = cx = dx = si = di = sp = bp = ip = flags = 0;
    cs = ss = es = ds = fs = gs = 0;
}

static inline void placeWordAt(uint16_t segment, uint16_t offset, uint16_t word) {
    *reinterpret_cast<uint16_t*>(CPU_REAL_TO_LINEAR(segment, offset)) = word;
}

#define CPU_PREFIX_REP   0xf3
#define CPU_PREFIX_REPNE 0xf2
#define CPU_PREFIX_LOCK  0xf0

#define CPU_PREFIX_OVERRIDE_CS  0x2E
#define CPU_PREFIX_OVERRIDE_SS  0x36
#define CPU_PREFIX_OVERRIDE_DS  0x3E
#define CPU_PREFIX_OVERRIDE_ES  0x26
#define CPU_PREFIX_OVERRIDE_FS  0x64
#define CPU_PREFIX_OVERRIDE_GS  0x65
#define CPU_PREFIX_OVERRIDE_OP  0x66
#define CPU_PREFIX_OVERRIDE_ADR 0x67

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

enum class cpu_instruction_prefix_t { NONE = 0, REP, REPNE, LOCK };
enum class cpu_segment_override_t { NONE = 0, CS, SS, DS, ES, FS, GS };

enum class opcode_addr_size_t { NONE, BYTE, WORD };
enum class opcode_immediate_size_t { NONE, MODRM, BYTE, WORD, DWORD };

struct cpu_instruction_t;

typedef void (*opcode_func_t)(cpu_instruction_t);

struct opcode_table_entry_t {
    uint8_t higherByte;
    uint8_t lowerByte;
    bool isWide;
    const char *name;
    opcode_addr_size_t addrSize;
    bool requiresModrm;
    bool requiresImmediate;
    opcode_immediate_size_t immediateSize;
    opcode_func_t func;
};


struct cpu_instruction_t {
    opcode_table_entry_t opcodeDef;

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
};

uintptr_t calculateModrmArgumentReference(cpu_instruction_t instruction) {
    switch (instruction.modrm.mod) {
        case 0:
            switch (instruction.modrm.rm) {
                case 0: return bx + si;
                case 1: return bx + di;
                case 2: return bp + si;
                case 3: return bp + di;
                case 4: return si;
                case 5: return si;
                case 6: return instruction.displacement;
                case 7: return bx;
            }
        case 1:
        case 2:
            switch (instruction.modrm.rm) {
                case 0: return bx + si + instruction.displacement;
                case 1: return bx + di + instruction.displacement;
                case 2: return bp + si + instruction.displacement;
                case 3: return bp + di + instruction.displacement;
                case 4: return si + instruction.displacement;
                case 5: return si + instruction.displacement;
                case 6: return bp + instruction.displacement;
                case 7: return bx + instruction.displacement;
            }
        case 3:
            switch (instruction.opcodeDef.addrSize) {
                case opcode_addr_size_t::NONE: panic_msg("calculateModrmArgumentReference() called on opcode with addr_size == NONE");
                case opcode_addr_size_t::BYTE: 
                    switch (instruction.modrm.rm) {
                        case 0: return reinterpret_cast<uintptr_t>(&ax) + sizeof(ax) - 1;
                        case 1: return reinterpret_cast<uintptr_t>(&cx) + sizeof(cx) - 1;
                        case 2: return reinterpret_cast<uintptr_t>(&dx) + sizeof(dx) - 1;
                        case 3: return reinterpret_cast<uintptr_t>(&bx) + sizeof(bx) - 1;
                        case 4: return reinterpret_cast<uintptr_t>(&ax) + sizeof(ax) - 1;
                        case 5: return reinterpret_cast<uintptr_t>(&cx) + sizeof(cx) - 1;
                        case 6: return reinterpret_cast<uintptr_t>(&dx) + sizeof(dx) - 1;
                        case 7: return reinterpret_cast<uintptr_t>(&bx) + sizeof(bx) - 1;
                    }
                case opcode_addr_size_t::WORD: 
                    switch (instruction.modrm.rm) {
                        case 0: return reinterpret_cast<uintptr_t>(&ax) + (instruction.addressSizePrefix ? 0 : sizeof(ax) - 2);
                        case 1: return reinterpret_cast<uintptr_t>(&cx) + (instruction.addressSizePrefix ? 0 : sizeof(cx) - 2);
                        case 2: return reinterpret_cast<uintptr_t>(&dx) + (instruction.addressSizePrefix ? 0 : sizeof(dx) - 2);
                        case 3: return reinterpret_cast<uintptr_t>(&bx) + (instruction.addressSizePrefix ? 0 : sizeof(bx) - 2);
                        case 4: return reinterpret_cast<uintptr_t>(&sp) + (instruction.addressSizePrefix ? 0 : sizeof(sp) - 2);
                        case 5: return reinterpret_cast<uintptr_t>(&bp) + (instruction.addressSizePrefix ? 0 : sizeof(bp) - 2);
                        case 6: return reinterpret_cast<uintptr_t>(&si) + (instruction.addressSizePrefix ? 0 : sizeof(si) - 2);
                        case 7: return reinterpret_cast<uintptr_t>(&di) + (instruction.addressSizePrefix ? 0 : sizeof(di) - 2);
                    }
            }
    }

    panic_msg("calculateModrmArgumentReference() reached end of function");
}

uint64_t getSizedValue(cpu_instruction_t instruction, uintptr_t from) {
    switch (instruction.opcodeDef.addrSize) {
        case opcode_addr_size_t::NONE: panic_msg("getSizedValue() called on opcode with addr_size == NONE");
        case opcode_addr_size_t::BYTE: return *reinterpret_cast<uint8_t*>(from);
        case opcode_addr_size_t::WORD: 
             return instruction.addressSizePrefix ? *reinterpret_cast<uint32_t*>(from) : *reinterpret_cast<uint16_t*>(from);
    }

    panic_msg("getSizedValue() reached end of function");
}

uint64_t signExtendedImmediate(cpu_instruction_t instruction) {
    switch (instruction.opcodeDef.immediateSize) {
        case opcode_immediate_size_t::NONE: panic_msg("signExtendedImmediate() called on opcode with immediate_size == NONE");
        case opcode_immediate_size_t::BYTE: return static_cast<int64_t>(static_cast<int8_t>(instruction.immediate));
        case opcode_immediate_size_t::WORD: return static_cast<int64_t>(static_cast<int16_t>(instruction.immediate));
        case opcode_immediate_size_t::DWORD: return static_cast<int64_t>(static_cast<int32_t>(instruction.immediate));
        case opcode_immediate_size_t::MODRM: 
            switch (instruction.opcodeDef.addrSize) {
                case opcode_addr_size_t::NONE: panic_msg("signExtendedImmediate() called on opcode with immediate_size == MODRM and addr_size == NONE");
                case opcode_addr_size_t::BYTE: return static_cast<int64_t>(static_cast<int8_t>(instruction.immediate));
                case opcode_addr_size_t::WORD: 
                    return instruction.addressSizePrefix 
                        ? static_cast<int64_t>(static_cast<int32_t>(instruction.immediate))
                        : static_cast<int64_t>(static_cast<int16_t>(instruction.immediate));
            }
    }

    panic_msg("signExtendedImmediate() reached end of function");
}

#define CPU_FLAG_CF (1 << 0)
#define CPU_FLAG_PF (1 << 2)
#define CPU_FLAG_AF (1 << 4)
#define CPU_FLAG_ZF (1 << 6)
#define CPU_FLAG_SF (1 << 7)
#define CPU_FLAG_TF (1 << 8)
#define CPU_FLAG_IF (1 << 9)
#define CPU_FLAG_DF (1 << 10)
#define CPU_FLAG_OF (1 << 11)

#define CARRY_IF(expr) ((expr) ? CPU_FLAG_CF : 0)
#define ZERO_IF(expr) ((expr) ? CPU_FLAG_ZF : 0)
#define OVERFLOW_IF(expr) ((expr) ? CPU_FLAG_OF : 0)
#define SIGN_IF(expr) ((expr) ? CPU_FLAG_SF : 0)
#define PARITY_IF(expr) ((expr) ? CPU_FLAG_PF : 0)

void opcode_mov(cpu_instruction_t instruction) {

}

void opcode_pushf(cpu_instruction_t instruction) {
    sp -= 2;
    placeWordAt(ss, sp, flags);
}

void opcode_alu_r_i8(cpu_instruction_t instruction) {
    uintptr_t lhsPtr = calculateModrmArgumentReference(instruction);
    uint64_t lhs = getSizedValue(instruction, lhsPtr);
    uint64_t rhs;

    if (flags & CPU_FLAG_AF) panic_msg("AF flag is not supported yet");

    // TODO parity flag support
    switch (instruction.modrm.regOrOpcode) {
        case 0: // ADD
            panic_msg("ADD not yet implemented");
            break;
        case 1: // OR
            panic_msg("OR not yet implemented");
            break;
        case 2: // ADC
            panic_msg("ADC not yet implemented");
            break;
        case 3: // SBB
            panic_msg("SBB not yet implemented");
            break;
        case 4: // AND
            panic_msg("AND not yet implemented");
            break;
        case 5: // SUB
            panic_msg("SUB not yet implemented");
            break;
        case 6: // XOR
            panic_msg("XOR not yet implemented");
            break;
        case 7: // CMP
            rhs = signExtendedImmediate(instruction);
            if (flags & CPU_FLAG_CF) lhs--;
            
            flags = CARRY_IF(lhs < rhs) |/* PARITY_IF(parityCheck(lhs - rhs)) | */ZERO_IF(lhs == rhs) | SIGN_IF(static_cast<int32_t>(lhs - rhs) < 0) |
                OVERFLOW_IF((lhs < rhs) ^ !!(flags & CPU_FLAG_CF));
                 
            break;
    }
}

void opcode_jnzr(cpu_instruction_t instruction) {
    ip += static_cast<int8_t>(instruction.immediate);
}

void opcode_cmp_rax_i16(cpu_instruction_t instruction) {
    uint64_t lhs = instruction.operandSizePrefix ? ax : static_cast<uint16_t>(ax);
    uint64_t rhs = signExtendedImmediate(instruction);

    if (flags & CPU_FLAG_CF) lhs--;
    
    flags = CARRY_IF(lhs < rhs) | ZERO_IF(lhs == rhs) | SIGN_IF(static_cast<int32_t>(lhs - rhs) < 0) |
        OVERFLOW_IF((lhs < rhs) ^ !!(flags & CPU_FLAG_CF));
}

void opcode_adc_rm8_r8(cpu_instruction_t instruction) {
    
}

struct opcode_table_entry_t opcodes[] = {
    { 0x3d, 0, false, "ADC_RM8_R8 ", opcode_addr_size_t::NONE, true,  false, opcode_immediate_size_t::NONE, opcode_adc_rm8_r8 },
    { 0x3d, 0, false, "CMP_rAX_I16", opcode_addr_size_t::NONE, false, true,  opcode_immediate_size_t::BYTE, opcode_cmp_rax_i16 },
    { 0x75, 0, false, "JNZ_R      ", opcode_addr_size_t::NONE, false, true,  opcode_immediate_size_t::BYTE, opcode_jnzr },
    { 0x80, 0, false, "ALU_R_I8   ", opcode_addr_size_t::BYTE, true,  true,  opcode_immediate_size_t::BYTE, opcode_alu_r_i8 },
    //{ 0x88, 0, false, "MOV       ", false, false, opcode_mov },
    { 0x9c, 0, false, "PUSHF      ", opcode_addr_size_t::NONE, false, false, opcode_immediate_size_t::NONE, opcode_pushf },
};

static void fetchAndExecute() {
    uint8_t *eip = reinterpret_cast<uint8_t*>(CPU_REAL_TO_LINEAR(cs, ip));
    uint8_t *startEip = eip;
    cpu_instruction_t instruction = {};

    switch (*eip++) {
        case CPU_PREFIX_LOCK:
            instruction.instructionPrefix = cpu_instruction_prefix_t::LOCK;
            break;
        case CPU_PREFIX_REP:
            instruction.instructionPrefix = cpu_instruction_prefix_t::REP;
            break;
        case CPU_PREFIX_REPNE:
            instruction.instructionPrefix = cpu_instruction_prefix_t::REPNE;
            break;
        default:
            eip--; // rolling back
            break;
    }

    if (*eip == CPU_PREFIX_OVERRIDE_ADR) {
        instruction.addressSizePrefix = true;
        eip++;
    }

    if (*eip == CPU_PREFIX_OVERRIDE_OP) {
        instruction.operandSizePrefix = true;
        eip++;
    }

    switch (*eip++) {
        case CPU_PREFIX_OVERRIDE_CS:
            instruction.segmentOverridePrefix = cpu_segment_override_t::CS;
            break;
        case CPU_PREFIX_OVERRIDE_DS:
            instruction.segmentOverridePrefix = cpu_segment_override_t::DS;
            break;
        case CPU_PREFIX_OVERRIDE_ES:
            instruction.segmentOverridePrefix = cpu_segment_override_t::ES;
            break;
        case CPU_PREFIX_OVERRIDE_SS:
            instruction.segmentOverridePrefix = cpu_segment_override_t::SS;
            break;
        case CPU_PREFIX_OVERRIDE_FS:
            instruction.segmentOverridePrefix = cpu_segment_override_t::FS;
            break;
        case CPU_PREFIX_OVERRIDE_GS:
            instruction.segmentOverridePrefix = cpu_segment_override_t::GS;
            break;
        default:
            eip--; // rolling back
            break;
    }

    uint8_t higherOpcode = *eip++;

    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(opcode_table_entry_t); i++) {
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

            if (opcodes[i].requiresModrm) {
                *reinterpret_cast<uint8_t*>(&instruction.modrm) = *eip++;

                switch (instruction.modrm.mod) {
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

            if (opcodes[i].requiresImmediate) {
                switch (opcodes[i].immediateSize) {
                    uint8_t b0, b1, b2, b3;
                    case opcode_immediate_size_t::NONE: 
                        printf("Opcode %s requires immediate value, but size is NONE", opcodes[i].name);
                        panic();
                        break;
                    case opcode_immediate_size_t::MODRM:
                        if (!opcodes[i].requiresModrm) {
                            printf("Opcode %s requires immediate value with MODRM size, but doesn't require MODRM", opcodes[i].name);
                            panic();
                        }
                        panic_msg("MODRM-sized immediate value is not yet implemented");
                        break;
                    case opcode_immediate_size_t::BYTE:
                        b0 = *eip++;
                        instruction.immediate = b0;
                        instruction.hasImmediate = true;
                        break;
                    case opcode_immediate_size_t::WORD:
                        b0 = *eip++;
                        b1 = *eip++;
                        instruction.immediate = (b0 << 8) | b1;
                        instruction.hasImmediate = true;
                        break;
                    case opcode_immediate_size_t::DWORD:
                        b0 = *eip++;
                        b1 = *eip++;
                        b2 = *eip++;
                        b3 = *eip++;
                        instruction.immediate = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
                        instruction.hasImmediate = true;
                        break;
                }
            }

            unsigned newIp = CPU_LINEAR_TO_OFFSET(cs, reinterpret_cast<uintptr_t>(eip));
            if (newIp >= CPU_SEGMENT_BOUNDARY) {
                printf("Code segment overflow\n");
                panic();
            }
            ip = newIp;
            
            printf("%08x    ", startEip);
            switch (instruction.instructionPrefix) {
                case cpu_instruction_prefix_t::NONE:  printf("      "); break;
                case cpu_instruction_prefix_t::REP:   printf("REP/E "); break;
                case cpu_instruction_prefix_t::REPNE: printf("REPNE "); break;
                case cpu_instruction_prefix_t::LOCK:  printf("LOCK  "); break;
            }
            if (instruction.addressSizePrefix) 
                printf("A32 ");
            else 
                printf("    ");
            if (instruction.operandSizePrefix) 
                printf("O32 ");
            else 
                printf("    ");
            switch (instruction.segmentOverridePrefix) {
                case cpu_segment_override_t::NONE: printf("   "); break;
                case cpu_segment_override_t::CS:   printf("CS "); break;
                case cpu_segment_override_t::DS:   printf("DS "); break;
                case cpu_segment_override_t::ES:   printf("ES "); break;
                case cpu_segment_override_t::FS:   printf("FS "); break;
                case cpu_segment_override_t::GS:   printf("GS "); break;
                case cpu_segment_override_t::SS:   printf("SS "); break;
            }
            printf(opcodes[i].name);
            if (opcodes[i].requiresModrm) {
                printf(" MOD/RM %d:%d:%d", instruction.modrm.mod, instruction.modrm.regOrOpcode, instruction.modrm.rm);
            }
            if (instruction.hasDisplacement) {
                printf(" DISP %04x", instruction.displacement);
            }
            if (instruction.hasImmediate) {
                printf(" IMM %08x", instruction.immediate);
            }
            printf("\n");
            opcodes[i].func(instruction);

            return;
        }
    }
    
    printf("Unknown opcode: %02x\n", higherOpcode);
    panic();
}

static void emulateInterrupt() {
    cs = int10handler.segment;
    ip = int10handler.offset;

    // using conventional memory for virtual stack placement
    ss = 0x7e0;
    sp = 0xfff0;

    // setting invalid return pointer ()
    placeWordAt(ss, sp, RETURN_OFFSET_MAGIC);
    placeWordAt(ss, sp + 2, RETURN_SEGMENT_MAGIC);

    do {
        fetchAndExecute();
    }
    while ((cs != RETURN_SEGMENT_MAGIC) && (ip != RETURN_OFFSET_MAGIC));
}

static void initMore(ivt_entry handler) {
    int10handler = handler;
    printf("vesa: setting mode 0x3, screen should wipe\n");

    eraseAllRegisters();
    ax = 3;
    emulateInterrupt();
}

void vesa::init() {
    printf("vesa: observing IVT\n");
    ivt_entry entry = *reinterpret_cast<ivt_entry*>(VESA_VIDEO_INTERRUPT * sizeof(ivt_entry));
    uintptr_t handlerPtr = CPU_REAL_TO_LINEAR(entry.segment, entry.offset);
    bool valid = HANDLER_PTR_LOOKS_LIKE_VALID(handlerPtr);
    printf("vesa: int 10h handler located in %04x:%04x, linear: %a, seems %s\n", entry.segment, entry.offset, handlerPtr, 
        valid ? "ok" : "not very valid");

    if (valid) {
        initMore(entry);
    }
}