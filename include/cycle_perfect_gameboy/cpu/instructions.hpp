#ifndef CYCLE_PERFECT_GAMEBOY_CPU_INSTRUCTIONS_HPP
#define CYCLE_PERFECT_GAMEBOY_CPU_INSTRUCTIONS_HPP

#include <cstdint>
#include <array>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/cpu/registers.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>
#include <cycle_perfect_gameboy/cpu/load_instructions.hpp>

namespace cycle_perfect_gameboy::cpu {

enum class InstructionType : std::uint8_t {
  Normal,  // Regular instruction
  CB,      // CB-prefixed instruction
};

struct Instruction {
  using Handler = auto (*)(Registers&, memory::MemoryInterface&, core::Address&) -> core::Cycles;
  
  std::uint8_t opcode;
  InstructionType type;
  const char* mnemonic;
  std::uint8_t length;
  Handler handler;
};

// Special instructions
constexpr auto nop([[maybe_unused]] Registers& regs, [[maybe_unused]] memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  return core::Cycles{4};
}

constexpr auto halt([[maybe_unused]] Registers& regs, [[maybe_unused]] memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // HALT stops the CPU until an interrupt occurs
  // For now, we just advance the PC and continue execution
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  return core::Cycles{4};
}

// Define the instruction table
constexpr std::array<Instruction, 256> instruction_table = {
  // 0x00 - 0x0F
  Instruction{0x00, InstructionType::Normal, "NOP", 1, nop},
  Instruction{0x01, InstructionType::Normal, "LD BC,nn", 3, ld_rp_nn},
  Instruction{0x02, InstructionType::Normal, "LD (BC),A", 1, ld_mem_a},
  Instruction{0x03, InstructionType::Normal, "INC BC", 1, nullptr}, // TODO
  Instruction{0x04, InstructionType::Normal, "INC B", 1, nullptr}, // TODO
  Instruction{0x05, InstructionType::Normal, "DEC B", 1, nullptr}, // TODO
  Instruction{0x06, InstructionType::Normal, "LD B,n", 2, ld_r_n},
  Instruction{0x07, InstructionType::Normal, "RLCA", 1, nullptr}, // TODO
  Instruction{0x08, InstructionType::Normal, "LD (nn),SP", 3, ld_nn_sp},
  Instruction{0x09, InstructionType::Normal, "ADD HL,BC", 1, nullptr}, // TODO
  Instruction{0x0A, InstructionType::Normal, "LD A,(BC)", 1, ld_a_mem},
  Instruction{0x0B, InstructionType::Normal, "DEC BC", 1, nullptr}, // TODO
  Instruction{0x0C, InstructionType::Normal, "INC C", 1, nullptr}, // TODO
  Instruction{0x0D, InstructionType::Normal, "DEC C", 1, nullptr}, // TODO
  Instruction{0x0E, InstructionType::Normal, "LD C,n", 2, ld_r_n},
  Instruction{0x0F, InstructionType::Normal, "RRCA", 1, nullptr}, // TODO

  // 0x10 - 0x1F
  Instruction{0x10, InstructionType::Normal, "STOP", 2, nullptr}, // TODO
  Instruction{0x11, InstructionType::Normal, "LD DE,nn", 3, ld_rp_nn},
  Instruction{0x12, InstructionType::Normal, "LD (DE),A", 1, ld_mem_a},
  Instruction{0x13, InstructionType::Normal, "INC DE", 1, nullptr}, // TODO
  Instruction{0x14, InstructionType::Normal, "INC D", 1, nullptr}, // TODO
  Instruction{0x15, InstructionType::Normal, "DEC D", 1, nullptr}, // TODO
  Instruction{0x16, InstructionType::Normal, "LD D,n", 2, ld_r_n},
  Instruction{0x17, InstructionType::Normal, "RLA", 1, nullptr}, // TODO
  Instruction{0x18, InstructionType::Normal, "JR n", 2, nullptr}, // TODO
  Instruction{0x19, InstructionType::Normal, "ADD HL,DE", 1, nullptr}, // TODO
  Instruction{0x1A, InstructionType::Normal, "LD A,(DE)", 1, ld_a_mem},
  Instruction{0x1B, InstructionType::Normal, "DEC DE", 1, nullptr}, // TODO
  Instruction{0x1C, InstructionType::Normal, "INC E", 1, nullptr}, // TODO
  Instruction{0x1D, InstructionType::Normal, "DEC E", 1, nullptr}, // TODO
  Instruction{0x1E, InstructionType::Normal, "LD E,n", 2, ld_r_n},
  Instruction{0x1F, InstructionType::Normal, "RRA", 1, nullptr}, // TODO

  // 0x20 - 0x2F
  Instruction{0x20, InstructionType::Normal, "JR NZ,n", 2, nullptr}, // TODO
  Instruction{0x21, InstructionType::Normal, "LD HL,nn", 3, ld_rp_nn},
  Instruction{0x22, InstructionType::Normal, "LD (HL+),A", 1, ld_mem_a},
  Instruction{0x23, InstructionType::Normal, "INC HL", 1, nullptr}, // TODO
  Instruction{0x24, InstructionType::Normal, "INC H", 1, nullptr}, // TODO
  Instruction{0x25, InstructionType::Normal, "DEC H", 1, nullptr}, // TODO
  Instruction{0x26, InstructionType::Normal, "LD H,n", 2, ld_r_n},
  Instruction{0x27, InstructionType::Normal, "DAA", 1, nullptr}, // TODO
  Instruction{0x28, InstructionType::Normal, "JR Z,n", 2, nullptr}, // TODO
  Instruction{0x29, InstructionType::Normal, "ADD HL,HL", 1, nullptr}, // TODO
  Instruction{0x2A, InstructionType::Normal, "LD A,(HL+)", 1, ld_a_mem},
  Instruction{0x2B, InstructionType::Normal, "DEC HL", 1, nullptr}, // TODO
  Instruction{0x2C, InstructionType::Normal, "INC L", 1, nullptr}, // TODO
  Instruction{0x2D, InstructionType::Normal, "DEC L", 1, nullptr}, // TODO
  Instruction{0x2E, InstructionType::Normal, "LD L,n", 2, ld_r_n},
  Instruction{0x2F, InstructionType::Normal, "CPL", 1, nullptr}, // TODO

  // 0x30 - 0x3F
  Instruction{0x30, InstructionType::Normal, "JR NC,n", 2, nullptr}, // TODO
  Instruction{0x31, InstructionType::Normal, "LD SP,nn", 3, ld_rp_nn},
  Instruction{0x32, InstructionType::Normal, "LD (HL-),A", 1, ld_mem_a},
  Instruction{0x33, InstructionType::Normal, "INC SP", 1, nullptr}, // TODO
  Instruction{0x34, InstructionType::Normal, "INC (HL)", 1, nullptr}, // TODO
  Instruction{0x35, InstructionType::Normal, "DEC (HL)", 1, nullptr}, // TODO
  Instruction{0x36, InstructionType::Normal, "LD (HL),n", 2, nullptr}, // TODO
  Instruction{0x37, InstructionType::Normal, "SCF", 1, nullptr}, // TODO
  Instruction{0x38, InstructionType::Normal, "JR C,n", 2, nullptr}, // TODO
  Instruction{0x39, InstructionType::Normal, "ADD HL,SP", 1, nullptr}, // TODO
  Instruction{0x3A, InstructionType::Normal, "LD A,(HL-)", 1, ld_a_mem},
  Instruction{0x3B, InstructionType::Normal, "DEC SP", 1, nullptr}, // TODO
  Instruction{0x3C, InstructionType::Normal, "INC A", 1, nullptr}, // TODO
  Instruction{0x3D, InstructionType::Normal, "DEC A", 1, nullptr}, // TODO
  Instruction{0x3E, InstructionType::Normal, "LD A,n", 2, ld_r_n},
  Instruction{0x3F, InstructionType::Normal, "CCF", 1, nullptr}, // TODO

  // 0x40 - 0x4F
  Instruction{0x40, InstructionType::Normal, "LD B,B", 1, ld_r_r},
  Instruction{0x41, InstructionType::Normal, "LD B,C", 1, ld_r_r},
  Instruction{0x42, InstructionType::Normal, "LD B,D", 1, ld_r_r},
  Instruction{0x43, InstructionType::Normal, "LD B,E", 1, ld_r_r},
  Instruction{0x44, InstructionType::Normal, "LD B,H", 1, ld_r_r},
  Instruction{0x45, InstructionType::Normal, "LD B,L", 1, ld_r_r},
  Instruction{0x46, InstructionType::Normal, "LD B,(HL)", 1, ld_r_r},
  Instruction{0x47, InstructionType::Normal, "LD B,A", 1, ld_r_r},
  Instruction{0x48, InstructionType::Normal, "LD C,B", 1, ld_r_r},
  Instruction{0x49, InstructionType::Normal, "LD C,C", 1, ld_r_r},
  Instruction{0x4A, InstructionType::Normal, "LD C,D", 1, ld_r_r},
  Instruction{0x4B, InstructionType::Normal, "LD C,E", 1, ld_r_r},
  Instruction{0x4C, InstructionType::Normal, "LD C,H", 1, ld_r_r},
  Instruction{0x4D, InstructionType::Normal, "LD C,L", 1, ld_r_r},
  Instruction{0x4E, InstructionType::Normal, "LD C,(HL)", 1, ld_r_r},
  Instruction{0x4F, InstructionType::Normal, "LD C,A", 1, ld_r_r},

  // 0x50 - 0x5F
  Instruction{0x50, InstructionType::Normal, "LD D,B", 1, ld_r_r},
  Instruction{0x51, InstructionType::Normal, "LD D,C", 1, ld_r_r},
  Instruction{0x52, InstructionType::Normal, "LD D,D", 1, ld_r_r},
  Instruction{0x53, InstructionType::Normal, "LD D,E", 1, ld_r_r},
  Instruction{0x54, InstructionType::Normal, "LD D,H", 1, ld_r_r},
  Instruction{0x55, InstructionType::Normal, "LD D,L", 1, ld_r_r},
  Instruction{0x56, InstructionType::Normal, "LD D,(HL)", 1, ld_r_r},
  Instruction{0x57, InstructionType::Normal, "LD D,A", 1, ld_r_r},
  Instruction{0x58, InstructionType::Normal, "LD E,B", 1, ld_r_r},
  Instruction{0x59, InstructionType::Normal, "LD E,C", 1, ld_r_r},
  Instruction{0x5A, InstructionType::Normal, "LD E,D", 1, ld_r_r},
  Instruction{0x5B, InstructionType::Normal, "LD E,E", 1, ld_r_r},
  Instruction{0x5C, InstructionType::Normal, "LD E,H", 1, ld_r_r},
  Instruction{0x5D, InstructionType::Normal, "LD E,L", 1, ld_r_r},
  Instruction{0x5E, InstructionType::Normal, "LD E,(HL)", 1, ld_r_r},
  Instruction{0x5F, InstructionType::Normal, "LD E,A", 1, ld_r_r},

  // 0x60 - 0x6F
  Instruction{0x60, InstructionType::Normal, "LD H,B", 1, ld_r_r},
  Instruction{0x61, InstructionType::Normal, "LD H,C", 1, ld_r_r},
  Instruction{0x62, InstructionType::Normal, "LD H,D", 1, ld_r_r},
  Instruction{0x63, InstructionType::Normal, "LD H,E", 1, ld_r_r},
  Instruction{0x64, InstructionType::Normal, "LD H,H", 1, ld_r_r},
  Instruction{0x65, InstructionType::Normal, "LD H,L", 1, ld_r_r},
  Instruction{0x66, InstructionType::Normal, "LD H,(HL)", 1, ld_r_r},
  Instruction{0x67, InstructionType::Normal, "LD H,A", 1, ld_r_r},
  Instruction{0x68, InstructionType::Normal, "LD L,B", 1, ld_r_r},
  Instruction{0x69, InstructionType::Normal, "LD L,C", 1, ld_r_r},
  Instruction{0x6A, InstructionType::Normal, "LD L,D", 1, ld_r_r},
  Instruction{0x6B, InstructionType::Normal, "LD L,E", 1, ld_r_r},
  Instruction{0x6C, InstructionType::Normal, "LD L,H", 1, ld_r_r},
  Instruction{0x6D, InstructionType::Normal, "LD L,L", 1, ld_r_r},
  Instruction{0x6E, InstructionType::Normal, "LD L,(HL)", 1, ld_r_r},
  Instruction{0x6F, InstructionType::Normal, "LD L,A", 1, ld_r_r},

  // 0x70 - 0x7F
  Instruction{0x70, InstructionType::Normal, "LD (HL),B", 1, ld_r_r},
  Instruction{0x71, InstructionType::Normal, "LD (HL),C", 1, ld_r_r},
  Instruction{0x72, InstructionType::Normal, "LD (HL),D", 1, ld_r_r},
  Instruction{0x73, InstructionType::Normal, "LD (HL),E", 1, ld_r_r},
  Instruction{0x74, InstructionType::Normal, "LD (HL),H", 1, ld_r_r},
  Instruction{0x75, InstructionType::Normal, "LD (HL),L", 1, ld_r_r},
  Instruction{0x76, InstructionType::Normal, "HALT", 1, halt},
  Instruction{0x77, InstructionType::Normal, "LD (HL),A", 1, ld_r_r},
  Instruction{0x78, InstructionType::Normal, "LD A,B", 1, ld_r_r},
  Instruction{0x79, InstructionType::Normal, "LD A,C", 1, ld_r_r},
  Instruction{0x7A, InstructionType::Normal, "LD A,D", 1, ld_r_r},
  Instruction{0x7B, InstructionType::Normal, "LD A,E", 1, ld_r_r},
  Instruction{0x7C, InstructionType::Normal, "LD A,H", 1, ld_r_r},
  Instruction{0x7D, InstructionType::Normal, "LD A,L", 1, ld_r_r},
  Instruction{0x7E, InstructionType::Normal, "LD A,(HL)", 1, ld_r_r},
  Instruction{0x7F, InstructionType::Normal, "LD A,A", 1, ld_r_r},

  // 0x80 - 0x8F (Add and substract instructions, to be implemented)
  Instruction{0x80, InstructionType::Normal, "ADD A,B", 1, nullptr}, // TODO
  Instruction{0x81, InstructionType::Normal, "ADD A,C", 1, nullptr}, // TODO
  Instruction{0x82, InstructionType::Normal, "ADD A,D", 1, nullptr}, // TODO
  Instruction{0x83, InstructionType::Normal, "ADD A,E", 1, nullptr}, // TODO
  Instruction{0x84, InstructionType::Normal, "ADD A,H", 1, nullptr}, // TODO
  Instruction{0x85, InstructionType::Normal, "ADD A,L", 1, nullptr}, // TODO
  Instruction{0x86, InstructionType::Normal, "ADD A,(HL)", 1, nullptr}, // TODO
  Instruction{0x87, InstructionType::Normal, "ADD A,A", 1, nullptr}, // TODO
  Instruction{0x88, InstructionType::Normal, "ADC A,B", 1, nullptr}, // TODO
  Instruction{0x89, InstructionType::Normal, "ADC A,C", 1, nullptr}, // TODO
  Instruction{0x8A, InstructionType::Normal, "ADC A,D", 1, nullptr}, // TODO
  Instruction{0x8B, InstructionType::Normal, "ADC A,E", 1, nullptr}, // TODO
  Instruction{0x8C, InstructionType::Normal, "ADC A,H", 1, nullptr}, // TODO
  Instruction{0x8D, InstructionType::Normal, "ADC A,L", 1, nullptr}, // TODO
  Instruction{0x8E, InstructionType::Normal, "ADC A,(HL)", 1, nullptr}, // TODO
  Instruction{0x8F, InstructionType::Normal, "ADC A,A", 1, nullptr}, // TODO

  // 0x90 - 0x9F (Subtraction instructions, to be implemented)
  Instruction{0x90, InstructionType::Normal, "SUB B", 1, nullptr}, // TODO
  Instruction{0x91, InstructionType::Normal, "SUB C", 1, nullptr}, // TODO
  Instruction{0x92, InstructionType::Normal, "SUB D", 1, nullptr}, // TODO
  Instruction{0x93, InstructionType::Normal, "SUB E", 1, nullptr}, // TODO
  Instruction{0x94, InstructionType::Normal, "SUB H", 1, nullptr}, // TODO
  Instruction{0x95, InstructionType::Normal, "SUB L", 1, nullptr}, // TODO
  Instruction{0x96, InstructionType::Normal, "SUB (HL)", 1, nullptr}, // TODO
  Instruction{0x97, InstructionType::Normal, "SUB A", 1, nullptr}, // TODO
  Instruction{0x98, InstructionType::Normal, "SBC A,B", 1, nullptr}, // TODO
  Instruction{0x99, InstructionType::Normal, "SBC A,C", 1, nullptr}, // TODO
  Instruction{0x9A, InstructionType::Normal, "SBC A,D", 1, nullptr}, // TODO
  Instruction{0x9B, InstructionType::Normal, "SBC A,E", 1, nullptr}, // TODO
  Instruction{0x9C, InstructionType::Normal, "SBC A,H", 1, nullptr}, // TODO
  Instruction{0x9D, InstructionType::Normal, "SBC A,L", 1, nullptr}, // TODO
  Instruction{0x9E, InstructionType::Normal, "SBC A,(HL)", 1, nullptr}, // TODO
  Instruction{0x9F, InstructionType::Normal, "SBC A,A", 1, nullptr}, // TODO

  // 0xA0 - 0xAF (Logical operations, to be implemented)
  Instruction{0xA0, InstructionType::Normal, "AND B", 1, nullptr}, // TODO
  Instruction{0xA1, InstructionType::Normal, "AND C", 1, nullptr}, // TODO
  Instruction{0xA2, InstructionType::Normal, "AND D", 1, nullptr}, // TODO
  Instruction{0xA3, InstructionType::Normal, "AND E", 1, nullptr}, // TODO
  Instruction{0xA4, InstructionType::Normal, "AND H", 1, nullptr}, // TODO
  Instruction{0xA5, InstructionType::Normal, "AND L", 1, nullptr}, // TODO
  Instruction{0xA6, InstructionType::Normal, "AND (HL)", 1, nullptr}, // TODO
  Instruction{0xA7, InstructionType::Normal, "AND A", 1, nullptr}, // TODO
  Instruction{0xA8, InstructionType::Normal, "XOR B", 1, nullptr}, // TODO
  Instruction{0xA9, InstructionType::Normal, "XOR C", 1, nullptr}, // TODO
  Instruction{0xAA, InstructionType::Normal, "XOR D", 1, nullptr}, // TODO
  Instruction{0xAB, InstructionType::Normal, "XOR E", 1, nullptr}, // TODO
  Instruction{0xAC, InstructionType::Normal, "XOR H", 1, nullptr}, // TODO
  Instruction{0xAD, InstructionType::Normal, "XOR L", 1, nullptr}, // TODO
  Instruction{0xAE, InstructionType::Normal, "XOR (HL)", 1, nullptr}, // TODO
  Instruction{0xAF, InstructionType::Normal, "XOR A", 1, nullptr}, // TODO

  // 0xB0 - 0xBF (Logical operations, to be implemented)
  Instruction{0xB0, InstructionType::Normal, "OR B", 1, nullptr}, // TODO
  Instruction{0xB1, InstructionType::Normal, "OR C", 1, nullptr}, // TODO
  Instruction{0xB2, InstructionType::Normal, "OR D", 1, nullptr}, // TODO
  Instruction{0xB3, InstructionType::Normal, "OR E", 1, nullptr}, // TODO
  Instruction{0xB4, InstructionType::Normal, "OR H", 1, nullptr}, // TODO
  Instruction{0xB5, InstructionType::Normal, "OR L", 1, nullptr}, // TODO
  Instruction{0xB6, InstructionType::Normal, "OR (HL)", 1, nullptr}, // TODO
  Instruction{0xB7, InstructionType::Normal, "OR A", 1, nullptr}, // TODO
  Instruction{0xB8, InstructionType::Normal, "CP B", 1, nullptr}, // TODO
  Instruction{0xB9, InstructionType::Normal, "CP C", 1, nullptr}, // TODO
  Instruction{0xBA, InstructionType::Normal, "CP D", 1, nullptr}, // TODO
  Instruction{0xBB, InstructionType::Normal, "CP E", 1, nullptr}, // TODO
  Instruction{0xBC, InstructionType::Normal, "CP H", 1, nullptr}, // TODO
  Instruction{0xBD, InstructionType::Normal, "CP L", 1, nullptr}, // TODO
  Instruction{0xBE, InstructionType::Normal, "CP (HL)", 1, nullptr}, // TODO
  Instruction{0xBF, InstructionType::Normal, "CP A", 1, nullptr}, // TODO

  // 0xC0 - 0xCF (Returns, calls, and stack operations)
  Instruction{0xC0, InstructionType::Normal, "RET NZ", 1, nullptr}, // TODO
  Instruction{0xC1, InstructionType::Normal, "POP BC", 1, pop_rp},
  Instruction{0xC2, InstructionType::Normal, "JP NZ,nn", 3, nullptr}, // TODO
  Instruction{0xC3, InstructionType::Normal, "JP nn", 3, nullptr}, // TODO
  Instruction{0xC4, InstructionType::Normal, "CALL NZ,nn", 3, nullptr}, // TODO
  Instruction{0xC5, InstructionType::Normal, "PUSH BC", 1, push_rp},
  Instruction{0xC6, InstructionType::Normal, "ADD A,n", 2, nullptr}, // TODO
  Instruction{0xC7, InstructionType::Normal, "RST 00H", 1, nullptr}, // TODO
  Instruction{0xC8, InstructionType::Normal, "RET Z", 1, nullptr}, // TODO
  Instruction{0xC9, InstructionType::Normal, "RET", 1, nullptr}, // TODO
  Instruction{0xCA, InstructionType::Normal, "JP Z,nn", 3, nullptr}, // TODO
  Instruction{0xCB, InstructionType::CB, "CB prefix", 1, nullptr}, // To be handled differently
  Instruction{0xCC, InstructionType::Normal, "CALL Z,nn", 3, nullptr}, // TODO
  Instruction{0xCD, InstructionType::Normal, "CALL nn", 3, nullptr}, // TODO
  Instruction{0xCE, InstructionType::Normal, "ADC A,n", 2, nullptr}, // TODO
  Instruction{0xCF, InstructionType::Normal, "RST 08H", 1, nullptr}, // TODO

  // 0xD0 - 0xDF (Returns, calls, and stack operations)
  Instruction{0xD0, InstructionType::Normal, "RET NC", 1, nullptr}, // TODO
  Instruction{0xD1, InstructionType::Normal, "POP DE", 1, pop_rp},
  Instruction{0xD2, InstructionType::Normal, "JP NC,nn", 3, nullptr}, // TODO
  Instruction{0xD3, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xD4, InstructionType::Normal, "CALL NC,nn", 3, nullptr}, // TODO
  Instruction{0xD5, InstructionType::Normal, "PUSH DE", 1, push_rp},
  Instruction{0xD6, InstructionType::Normal, "SUB n", 2, nullptr}, // TODO
  Instruction{0xD7, InstructionType::Normal, "RST 10H", 1, nullptr}, // TODO
  Instruction{0xD8, InstructionType::Normal, "RET C", 1, nullptr}, // TODO
  Instruction{0xD9, InstructionType::Normal, "RETI", 1, nullptr}, // TODO
  Instruction{0xDA, InstructionType::Normal, "JP C,nn", 3, nullptr}, // TODO
  Instruction{0xDB, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xDC, InstructionType::Normal, "CALL C,nn", 3, nullptr}, // TODO
  Instruction{0xDD, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xDE, InstructionType::Normal, "SBC A,n", 2, nullptr}, // TODO
  Instruction{0xDF, InstructionType::Normal, "RST 18H", 1, nullptr}, // TODO

  // 0xE0 - 0xEF (I/O operations, stack operations, and other instructions)
  Instruction{0xE0, InstructionType::Normal, "LDH (n),A", 2, ld_io_a},
  Instruction{0xE1, InstructionType::Normal, "POP HL", 1, pop_rp},
  Instruction{0xE2, InstructionType::Normal, "LD (C),A", 1, ld_io_a},
  Instruction{0xE3, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xE4, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xE5, InstructionType::Normal, "PUSH HL", 1, push_rp},
  Instruction{0xE6, InstructionType::Normal, "AND n", 2, nullptr}, // TODO
  Instruction{0xE7, InstructionType::Normal, "RST 20H", 1, nullptr}, // TODO
  Instruction{0xE8, InstructionType::Normal, "ADD SP,n", 2, nullptr}, // TODO
  Instruction{0xE9, InstructionType::Normal, "JP (HL)", 1, nullptr}, // TODO
  Instruction{0xEA, InstructionType::Normal, "LD (nn),A", 3, ld_a_abs},
  Instruction{0xEB, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xEC, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xED, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xEE, InstructionType::Normal, "XOR n", 2, nullptr}, // TODO
  Instruction{0xEF, InstructionType::Normal, "RST 28H", 1, nullptr}, // TODO

  // 0xF0 - 0xFF (I/O operations, stack operations, and other instructions)
  Instruction{0xF0, InstructionType::Normal, "LDH A,(n)", 2, ld_io_a},
  Instruction{0xF1, InstructionType::Normal, "POP AF", 1, pop_rp},
  Instruction{0xF2, InstructionType::Normal, "LD A,(C)", 1, ld_io_a},
  Instruction{0xF3, InstructionType::Normal, "DI", 1, nullptr}, // TODO
  Instruction{0xF4, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xF5, InstructionType::Normal, "PUSH AF", 1, push_rp},
  Instruction{0xF6, InstructionType::Normal, "OR n", 2, nullptr}, // TODO
  Instruction{0xF7, InstructionType::Normal, "RST 30H", 1, nullptr}, // TODO
  Instruction{0xF8, InstructionType::Normal, "LD HL,SP+n", 2, ld_hl_sp_n},
  Instruction{0xF9, InstructionType::Normal, "LD SP,HL", 1, ld_sp_hl},
  Instruction{0xFA, InstructionType::Normal, "LD A,(nn)", 3, ld_a_abs},
  Instruction{0xFB, InstructionType::Normal, "EI", 1, nullptr}, // TODO
  Instruction{0xFC, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xFD, InstructionType::Normal, "Invalid", 1, nullptr}, // Invalid instruction
  Instruction{0xFE, InstructionType::Normal, "CP n", 2, nullptr}, // TODO
  Instruction{0xFF, InstructionType::Normal, "RST 38H", 1, nullptr}, // TODO
};

} // namespace cycle_perfect_gameboy::cpu

#endif // CYCLE_PERFECT_GAMEBOY_CPU_INSTRUCTIONS_HPP