#ifndef CYCLE_PERFECT_GAMEBOY_CPU_LOAD_INSTRUCTIONS_HPP
#define CYCLE_PERFECT_GAMEBOY_CPU_LOAD_INSTRUCTIONS_HPP

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/cpu/registers.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::cpu {

// Load immediate 8-bit value into register
// Opcode: 0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E (LD r, n)
constexpr auto ld_r_n(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get operand from memory
  const auto value = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)});
  
  // Get the register to load into from the opcode
  const auto opcode = mem.read(pc);
  
  // Extract the register index from bits 3-5 of the opcode
  // 0x06 -> B, 0x0E -> C, 0x16 -> D, 0x1E -> E, 0x26 -> H, 0x2E -> L, 0x3E -> A
  const auto reg_idx = (opcode >> 3) & 0x07;
  
  // Load the value into the corresponding register
  switch (reg_idx) {
    case 0: regs.set_b(value); break;
    case 1: regs.set_c(value); break;
    case 2: regs.set_d(value); break;
    case 3: regs.set_e(value); break;
    case 4: regs.set_h(value); break;
    case 5: regs.set_l(value); break;
    case 7: regs.set_a(value); break;
    default: break; // Should never happen
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 2)};
  
  // Return the number of cycles
  return core::Cycles{8};
}

// Load register into register
// Opcodes: 0x40-0x7F except 0x76 (HALT)
constexpr auto ld_r_r(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  // Extract source register index from bits 0-2
  const auto src_reg = opcode & 0x07;
  
  // Extract destination register index from bits 3-5
  const auto dst_reg = (opcode >> 3) & 0x07;
  
  // Read value from source register
  std::uint8_t value = 0;
  switch (src_reg) {
    case 0: value = regs.get_b(); break;
    case 1: value = regs.get_c(); break;
    case 2: value = regs.get_d(); break;
    case 3: value = regs.get_e(); break;
    case 4: value = regs.get_h(); break;
    case 5: value = regs.get_l(); break;
    case 6: value = mem.read(regs.get_hl().to_address()); break; // (HL)
    case 7: value = regs.get_a(); break;
    default: break; // Should never happen
  }
  
  // Write value to destination register
  switch (dst_reg) {
    case 0: regs.set_b(value); break;
    case 1: regs.set_c(value); break;
    case 2: regs.set_d(value); break;
    case 3: regs.set_e(value); break;
    case 4: regs.set_h(value); break;
    case 5: regs.set_l(value); break;
    case 6: mem.write(regs.get_hl().to_address(), value); break; // (HL)
    case 7: regs.set_a(value); break;
    default: break; // Should never happen
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};

  // Return the number of cycles (1 M-cycle normally, 2 M-cycles when (HL) is involved)
  if (src_reg == 6 || dst_reg == 6) {
    return core::Cycles{8}; // 2 M-cycles when (HL) is involved
  }
  return core::Cycles{4}; // 1 M-cycle for register-to-register
}

// Load value from memory into register
// Opcodes: 0x0A (LD A,(BC)), 0x1A (LD A,(DE)), 0x2A (LD A,(HL+)), 0x3A (LD A,(HL-))
constexpr auto ld_a_mem(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  core::Address src_addr{0};
  bool increment_hl = false;
  bool decrement_hl = false;
  
  // Determine the source address
  switch (opcode) {
    case 0x0A: // LD A,(BC)
      src_addr = regs.get_bc().to_address();
      break;
    case 0x1A: // LD A,(DE)
      src_addr = regs.get_de().to_address();
      break;
    case 0x2A: // LD A,(HL+)
      src_addr = regs.get_hl().to_address();
      increment_hl = true;
      break;
    case 0x3A: // LD A,(HL-)
      src_addr = regs.get_hl().to_address();
      decrement_hl = true;
      break;
    default:
      break; // Should never happen
  }
  
  // Read from the source address
  const auto value = mem.read(src_addr);
  
  // Store in A
  regs.set_a(value);
  
  // Handle HL increment/decrement
  if (increment_hl) {
    regs.set_hl(core::RegisterPair{static_cast<std::uint16_t>(regs.get_hl().value() + 1)});
  } else if (decrement_hl) {
    regs.set_hl(core::RegisterPair{static_cast<std::uint16_t>(regs.get_hl().value() - 1)});
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  
  // Return the number of cycles
  return core::Cycles{8}; // 2 M-cycles
}

// Store register in memory
// Opcodes: 0x02 (LD (BC),A), 0x12 (LD (DE),A), 0x22 (LD (HL+),A), 0x32 (LD (HL-),A)
constexpr auto ld_mem_a(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  core::Address dst_addr{0};
  bool increment_hl = false;
  bool decrement_hl = false;
  
  // Determine the destination address
  switch (opcode) {
    case 0x02: // LD (BC),A
      dst_addr = regs.get_bc().to_address();
      break;
    case 0x12: // LD (DE),A
      dst_addr = regs.get_de().to_address();
      break;
    case 0x22: // LD (HL+),A
      dst_addr = regs.get_hl().to_address();
      increment_hl = true;
      break;
    case 0x32: // LD (HL-),A
      dst_addr = regs.get_hl().to_address();
      decrement_hl = true;
      break;
    default:
      break; // Should never happen
  }
  
  // Write to the destination address
  mem.write(dst_addr, regs.get_a());
  
  // Handle HL increment/decrement
  if (increment_hl) {
    regs.set_hl(core::RegisterPair{static_cast<std::uint16_t>(regs.get_hl().value() + 1)});
  } else if (decrement_hl) {
    regs.set_hl(core::RegisterPair{static_cast<std::uint16_t>(regs.get_hl().value() - 1)});
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  
  // Return the number of cycles
  return core::Cycles{8}; // 2 M-cycles
}

// Load A from absolute address or store A to absolute address
// Opcodes: 0xEA (LD (nn),A), 0xFA (LD A,(nn))
constexpr auto ld_a_abs(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  // Get the address from the next two bytes (little endian)
  const auto low_byte = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)});
  const auto high_byte = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 2)});
  const auto addr = static_cast<std::uint16_t>((high_byte << 8) | low_byte);
  
  // Perform load/store based on opcode
  if (opcode == 0xEA) { // LD (nn),A
    // Write A to memory at absolute address
    mem.write(core::Address{addr}, regs.get_a());
  } else if (opcode == 0xFA) { // LD A,(nn)
    // Read from absolute address into A
    regs.set_a(mem.read(core::Address{addr}));
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 3)};
  
  // Return the number of cycles
  return core::Cycles{16}; // 4 M-cycles
}

// Load 16-bit immediate value into register pair
// Opcodes: 0x01 (LD BC,nn), 0x11 (LD DE,nn), 0x21 (LD HL,nn), 0x31 (LD SP,nn)
constexpr auto ld_rp_nn(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  // Get the 16-bit immediate value (little endian)
  const auto low_byte = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)});
  const auto high_byte = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 2)});
  const auto value = static_cast<std::uint16_t>((high_byte << 8) | low_byte);
  
  // Determine which register pair to load into
  switch (opcode) {
    case 0x01: // LD BC,nn
      regs.set_bc(core::RegisterPair{value});
      break;
    case 0x11: // LD DE,nn
      regs.set_de(core::RegisterPair{value});
      break;
    case 0x21: // LD HL,nn
      regs.set_hl(core::RegisterPair{value});
      break;
    case 0x31: // LD SP,nn
      regs.set_sp(core::RegisterPair{value});
      break;
    default:
      break; // Should never happen
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 3)};
  
  // Return the number of cycles
  return core::Cycles{12}; // 3 M-cycles
}

// Load SP into memory at address nn
// Opcode: 0x08 (LD (nn),SP)
constexpr auto ld_nn_sp(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the address from the immediate value (little endian)
  const auto low_byte = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)});
  const auto high_byte = mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 2)});
  const auto address = static_cast<std::uint16_t>((high_byte << 8) | low_byte);
  
  // Get the SP value
  const auto sp_value = regs.get_sp().value();
  
  // Write the SP value to the address (little endian)
  mem.write(core::Address{address}, static_cast<std::uint8_t>(sp_value & 0xFF));
  mem.write(core::Address{static_cast<std::uint16_t>(address + 1)}, static_cast<std::uint8_t>(sp_value >> 8));
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 3)};
  
  // Return the number of cycles
  return core::Cycles{20}; // 5 M-cycles
}

// Load HL into SP
// Opcode: 0xF9 (LD SP,HL)
constexpr auto ld_sp_hl(Registers& regs, [[maybe_unused]] memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Load HL into SP
  regs.set_sp(regs.get_hl());
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  
  // Return the number of cycles
  return core::Cycles{8}; // 2 M-cycles
}

// Push register pair onto stack
// Opcodes: 0xC5 (PUSH BC), 0xD5 (PUSH DE), 0xE5 (PUSH HL), 0xF5 (PUSH AF)
constexpr auto push_rp(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  // Determine which register pair to push
  std::uint16_t value = 0;
  switch (opcode) {
    case 0xC5: // PUSH BC
      value = regs.get_bc().value();
      break;
    case 0xD5: // PUSH DE
      value = regs.get_de().value();
      break;
    case 0xE5: // PUSH HL
      value = regs.get_hl().value();
      break;
    case 0xF5: // PUSH AF
      value = regs.get_af().value();
      break;
    default:
      break; // Should never happen
  }
  
  // Decrement SP
  regs.set_sp(core::RegisterPair{static_cast<std::uint16_t>(regs.get_sp().value() - 1)});
  
  // Push high byte
  mem.write(regs.get_sp().to_address(), static_cast<std::uint8_t>(value >> 8));
  
  // Decrement SP
  regs.set_sp(core::RegisterPair{static_cast<std::uint16_t>(regs.get_sp().value() - 1)});
  
  // Push low byte
  mem.write(regs.get_sp().to_address(), static_cast<std::uint8_t>(value & 0xFF));
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  
  // Return the number of cycles
  return core::Cycles{16}; // 4 M-cycles
}

// Pop register pair from stack
// Opcodes: 0xC1 (POP BC), 0xD1 (POP DE), 0xE1 (POP HL), 0xF1 (POP AF)
constexpr auto pop_rp(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  // Pop value from stack (little endian)
  const auto low_byte = mem.read(regs.get_sp().to_address());
  
  // Increment SP
  regs.set_sp(core::RegisterPair{static_cast<std::uint16_t>(regs.get_sp().value() + 1)});
  
  const auto high_byte = mem.read(regs.get_sp().to_address());
  
  // Increment SP
  regs.set_sp(core::RegisterPair{static_cast<std::uint16_t>(regs.get_sp().value() + 1)});
  
  // Combine bytes into 16-bit value
  const auto value = static_cast<std::uint16_t>((high_byte << 8) | low_byte);
  
  // Determine which register pair to pop into
  switch (opcode) {
    case 0xC1: // POP BC
      regs.set_bc(core::RegisterPair{value});
      break;
    case 0xD1: // POP DE
      regs.set_de(core::RegisterPair{value});
      break;
    case 0xE1: // POP HL
      regs.set_hl(core::RegisterPair{value});
      break;
    case 0xF1: // POP AF
      // Ensure the lower 4 bits of F are always 0
      regs.set_af(core::RegisterPair{static_cast<std::uint16_t>(value & 0xFFF0)});
      break;
    default:
      break; // Should never happen
  }
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  
  // Return the number of cycles
  return core::Cycles{12}; // 3 M-cycles
}

// Load from offset address relative to HL or I/O ports
// Opcodes: 0xE0 (LD (FF00+n),A), 0xF0 (LD A,(FF00+n)),
//          0xE2 (LD (FF00+C),A), 0xF2 (LD A,(FF00+C))
constexpr auto ld_io_a(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the opcode
  const auto opcode = mem.read(pc);
  
  core::Address addr{0};
  bool is_write = false;
  
  // Determine operation type and address
  switch (opcode) {
    case 0xE0: // LD (FF00+n),A
      addr = core::Address{static_cast<std::uint16_t>(0xFF00 + mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)}))};
      is_write = true;
      break;
    case 0xF0: // LD A,(FF00+n)
      addr = core::Address{static_cast<std::uint16_t>(0xFF00 + mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)}))};
      is_write = false;
      break;
    case 0xE2: // LD (FF00+C),A
      addr = core::Address{static_cast<std::uint16_t>(0xFF00 + regs.get_c())};
      is_write = true;
      break;
    case 0xF2: // LD A,(FF00+C)
      addr = core::Address{static_cast<std::uint16_t>(0xFF00 + regs.get_c())};
      is_write = false;
      break;
    default:
      break; // Should never happen
  }
  
  // Perform the operation
  if (is_write) {
    mem.write(addr, regs.get_a());
  } else {
    regs.set_a(mem.read(addr));
  }
  
  // Update program counter
  if (opcode == 0xE0 || opcode == 0xF0) {
    // Instructions with immediate byte operand
    pc = core::Address{static_cast<std::uint16_t>(pc.value() + 2)};
  } else {
    // Instructions using C register
    pc = core::Address{static_cast<std::uint16_t>(pc.value() + 1)};
  }
  
  // Return the number of cycles
  return core::Cycles{12}; // 3 M-cycles (for all these instructions)
}

// Load 16-bit immediate into HL with SP offset
// Opcode: 0xF8 (LD HL,SP+n)
constexpr auto ld_hl_sp_n(Registers& regs, memory::MemoryInterface& mem, core::Address& pc) -> core::Cycles {
  // Get the signed offset
  const auto offset = static_cast<std::int8_t>(mem.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)}));
  
  // Get the SP value
  const auto sp = regs.get_sp().value();
  
  // Calculate the result
  const auto result = static_cast<std::uint16_t>(sp + offset);
  
  // Set HL
  regs.set_hl(core::RegisterPair{result});
  
  // Set flags (reset Z and N, set H and C based on addition)
  regs.set_zero_flag(false);
  regs.set_subtract_flag(false);
  
  // Set H if carry from bit 3
  regs.set_half_carry_flag((sp & 0x0F) + (offset & 0x0F) > 0x0F);
  
  // Set C if carry from bit 7
  regs.set_carry_flag((sp & 0xFF) + (offset & 0xFF) > 0xFF);
  
  // Update program counter
  pc = core::Address{static_cast<std::uint16_t>(pc.value() + 2)};
  
  // Return the number of cycles
  return core::Cycles{12}; // 3 M-cycles
}

} // namespace cycle_perfect_gameboy::cpu

#endif // CYCLE_PERFECT_GAMEBOY_CPU_LOAD_INSTRUCTIONS_HPP