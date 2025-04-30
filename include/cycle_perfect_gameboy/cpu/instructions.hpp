#ifndef CYCLE_PERFECT_GAMEBOY_CPU_INSTRUCTIONS_HPP
#define CYCLE_PERFECT_GAMEBOY_CPU_INSTRUCTIONS_HPP

#include <cstdint>
#include <array>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/cpu/registers.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::cpu {

enum class InstructionType : std::uint8_t {
  Normal,  // Regular instruction
  CB,      // CB-prefixed instruction
};

struct Instruction {
  using Handler = auto (*)(Registers&, const memory::MemoryInterface&, core::Address) -> core::Cycles;
  
  std::uint8_t opcode;
  InstructionType type;
  const char* mnemonic;
  std::uint8_t length;
  Handler handler;
};

// Load 8-bit Immediate instruction
[[nodiscard]] constexpr auto ld_r_n(Registers& registers, const memory::MemoryInterface& memory, core::Address pc) -> core::Cycles {
  const auto n = memory.read(pc + core::Address{1});
  // This is a placeholder to be expanded later with actual implementation that modifies the correct register
  // For now, we'll just load into A register as an example
  registers.set_a(n);
  return core::Cycles{2};
}

// Define the instruction table
constexpr std::array<Instruction, 256> instruction_table = {
  // This is a placeholder table that will be filled with all 256 instructions
  Instruction{0x06, InstructionType::Normal, "LD B,n", 2, ld_r_n},
  // More instructions to be added
};

} // namespace cycle_perfect_gameboy::cpu

#endif // CYCLE_PERFECT_GAMEBOY_CPU_INSTRUCTIONS_HPP