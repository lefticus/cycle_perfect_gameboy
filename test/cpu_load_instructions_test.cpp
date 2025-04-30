#include <catch2/catch_test_macros.hpp>

#include "cycle_perfect_gameboy/cpu/load_instructions.hpp"
#include "cycle_perfect_gameboy/memory/memory_interface.hpp"
#include "cycle_perfect_gameboy/core/types.hpp"
#include "cycle_perfect_gameboy/cpu/registers.hpp"

#include <array>
#include <cstdint>
#include <initializer_list>

// Mock memory for testing
class MockMemory : public cycle_perfect_gameboy::memory::MemoryInterface {
private:
  std::array<std::uint8_t, 0x10000> memory_{}; // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

public:
  MockMemory() {
    // Initialize memory to zeros
    memory_.fill(0);
  }

  // Make sure we're returning numeric values, not characters
  [[nodiscard]] auto read(cycle_perfect_gameboy::core::Address addr) const -> std::uint8_t override {
    // Store the value to ensure it's interpreted as a number
    const std::uint8_t value = memory_[addr.value()];
    return value;
  }

  void write(cycle_perfect_gameboy::core::Address addr, std::uint8_t value) override {
    // Make sure we're writing numeric values
    const std::uint8_t numeric_value = static_cast<std::uint8_t>(value);
    memory_[addr.value()] = numeric_value;
  }

  // Helper to set up memory for tests
  void set_memory(cycle_perfect_gameboy::core::Address addr, std::initializer_list<std::uint8_t> values) { // NOLINT(readability-named-parameter)
    std::uint16_t offset = 0;
    for (auto value : values) {
      // Make sure values are treated as numeric
      const std::uint8_t numeric_value = static_cast<std::uint8_t>(value);
      memory_[addr.value() + offset++] = numeric_value;
    }
  }
};

TEST_CASE("8-bit register load instructions", "[cpu][load]") { // NOLINT
  using namespace cycle_perfect_gameboy; // NOLINT
  
  MockMemory memory;
  cpu::Registers registers;
  
  SECTION("LD r,n instructions load 8-bit immediate values into registers") { // NOLINT
    // Set up test cases
    // NOLINT below for all magic numbers in test data
    memory.set_memory(core::Address{0x0000}, {0x06, 0x42}); // LD B,n // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x0E, 0x84}); // LD C,n // NOLINT
    memory.set_memory(core::Address{0x0004}, {0x16, 0xA1}); // LD D,n // NOLINT
    memory.set_memory(core::Address{0x0006}, {0x1E, 0x37}); // LD E,n // NOLINT
    memory.set_memory(core::Address{0x0008}, {0x26, 0xF5}); // LD H,n // NOLINT
    memory.set_memory(core::Address{0x000A}, {0x2E, 0x19}); // LD L,n // NOLINT
    memory.set_memory(core::Address{0x000C}, {0x3E, 0xCB}); // LD A,n // NOLINT
    
    // Execute LD B,n
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_r_n(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_b() == 0x42); // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD C,n
    cycles = cpu::ld_r_n(registers, memory, program_counter);
    REQUIRE(registers.get_c() == 0x84); // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD D,n
    cycles = cpu::ld_r_n(registers, memory, program_counter);
    REQUIRE(registers.get_d() == 0xA1); // NOLINT
    REQUIRE(program_counter.value() == 0x0006); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD E,n
    cycles = cpu::ld_r_n(registers, memory, program_counter);
    REQUIRE(registers.get_e() == 0x37); // NOLINT
    REQUIRE(program_counter.value() == 0x0008); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD H,n
    cycles = cpu::ld_r_n(registers, memory, program_counter);
    REQUIRE(registers.get_h() == 0xF5); // NOLINT
    REQUIRE(program_counter.value() == 0x000A); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD L,n
    cycles = cpu::ld_r_n(registers, memory, program_counter);
    REQUIRE(registers.get_l() == 0x19); // NOLINT
    REQUIRE(program_counter.value() == 0x000C); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD A,n
    cycles = cpu::ld_r_n(registers, memory, program_counter);
    REQUIRE(registers.get_a() == 0xCB); // NOLINT
    REQUIRE(program_counter.value() == 0x000E); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
  
  SECTION("LD r,r instructions copy between registers") { // NOLINT
    // Set up registers - NOLINT for all magic numbers in test data
    registers.set_a(0x12); // NOLINT
    registers.set_b(0x34); // NOLINT
    registers.set_c(0x56); // NOLINT
    registers.set_d(0x78); // NOLINT
    registers.set_e(0x9A); // NOLINT
    registers.set_h(0xBC); // NOLINT
    registers.set_l(0xDE); // NOLINT
    
    // Set up memory opcodes for LD instructions
    memory.set_memory(core::Address{0x0000}, {0x47}); // LD B,A // NOLINT
    memory.set_memory(core::Address{0x0001}, {0x48}); // LD C,B // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x52}); // LD D,D (no change) // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x5F}); // LD E,A // NOLINT
    
    // Execute LD B,A
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_r_r(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_b() == 0x12); // B = A // NOLINT
    REQUIRE(program_counter.value() == 0x0001); // NOLINT
    REQUIRE(cycles.value() == 4); // NOLINT
    
    // Execute LD C,B
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    REQUIRE(registers.get_c() == 0x12); // C = B // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 4); // NOLINT
    
    // Execute LD D,D (should be a no-op)
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    REQUIRE(registers.get_d() == 0x78); // D unchanged // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // NOLINT
    REQUIRE(cycles.value() == 4); // NOLINT
    
    // Execute LD E,A
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    REQUIRE(registers.get_e() == 0x12); // E = A // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 4); // NOLINT
  }

  SECTION("LD r,(HL) and LD (HL),r instructions work correctly") { // NOLINT
    // Set up registers and memory
    registers.set_a(0x12); // NOLINT
    registers.set_b(0x34); // NOLINT
    registers.set_c(0x56); // NOLINT
    registers.set_hl(core::RegisterPair{0x1000}); // NOLINT
    
    // Write a value to the memory location pointed to by HL
    memory.write(core::Address{0x1000}, 0x42); // NOLINT
    
    // Set up memory opcodes for LD instructions
    memory.set_memory(core::Address{0x0000}, {0x46}); // LD B,(HL) // NOLINT
    memory.set_memory(core::Address{0x0001}, {0x4E}); // LD C,(HL) // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x77}); // LD (HL),A // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x71}); // LD (HL),C // NOLINT
    
    // Execute LD B,(HL)
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_r_r(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_b() == 0x42); // B = (HL) // NOLINT
    REQUIRE(program_counter.value() == 0x0001); // NOLINT
    REQUIRE(cycles.value() == 8); // 8 cycles for memory access // NOLINT
    
    // Execute LD C,(HL)
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    REQUIRE(registers.get_c() == 0x42); // C = (HL) // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD (HL),A
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    REQUIRE(memory.read(core::Address{0x1000}) == 0x12); // (HL) = A // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD (HL),C
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    REQUIRE(memory.read(core::Address{0x1000}) == 0x56); // (HL) = C // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
  
  SECTION("16-bit load instructions work correctly") { // NOLINT
    // Set up memory opcodes for LD rp,nn instructions
    memory.set_memory(core::Address{0x0000}, {0x01, 0x34, 0x12}); // LD BC,nn // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x11, 0x78, 0x56}); // LD DE,nn // NOLINT
    memory.set_memory(core::Address{0x0006}, {0x21, 0xBC, 0x9A}); // LD HL,nn // NOLINT
    memory.set_memory(core::Address{0x0009}, {0x31, 0xF0, 0xDE}); // LD SP,nn // NOLINT
    
    // Execute LD BC,nn
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_rp_nn(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_bc().value() == 0x1234); // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Execute LD DE,nn
    cycles = cpu::ld_rp_nn(registers, memory, program_counter);
    REQUIRE(registers.get_de().value() == 0x5678); // NOLINT
    REQUIRE(program_counter.value() == 0x0006); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Execute LD HL,nn
    cycles = cpu::ld_rp_nn(registers, memory, program_counter);
    REQUIRE(registers.get_hl().value() == 0x9ABC); // NOLINT
    REQUIRE(program_counter.value() == 0x0009); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Execute LD SP,nn
    cycles = cpu::ld_rp_nn(registers, memory, program_counter);
    REQUIRE(registers.get_sp().value() == 0xDEF0); // NOLINT
    REQUIRE(program_counter.value() == 0x000C); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
  }

  SECTION("Stack operations (PUSH and POP) work correctly") { // NOLINT
    // Setup registers
    registers.set_sp(core::RegisterPair{0xFFFE}); // NOLINT
    registers.set_af(core::RegisterPair{0x1234}); // NOLINT
    registers.set_bc(core::RegisterPair{0x5678}); // NOLINT
    registers.set_de(core::RegisterPair{0x9ABC}); // NOLINT
    registers.set_hl(core::RegisterPair{0xDEF0}); // NOLINT
    
    // Set up memory opcodes for PUSH/POP instructions
    memory.set_memory(core::Address{0x0000}, {0xF5}); // PUSH AF // NOLINT
    memory.set_memory(core::Address{0x0001}, {0xC5}); // PUSH BC // NOLINT
    memory.set_memory(core::Address{0x0002}, {0xD5}); // PUSH DE // NOLINT
    memory.set_memory(core::Address{0x0003}, {0xE5}); // PUSH HL // NOLINT
    memory.set_memory(core::Address{0x0004}, {0xE1}); // POP HL // NOLINT
    memory.set_memory(core::Address{0x0005}, {0xD1}); // POP DE // NOLINT
    memory.set_memory(core::Address{0x0006}, {0xC1}); // POP BC // NOLINT
    memory.set_memory(core::Address{0x0007}, {0xF1}); // POP AF // NOLINT
    
    // Execute PUSH AF
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::push_rp(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_sp().value() == 0xFFFC); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFFC}) == 0x34); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFFD}) == 0x12); // NOLINT
    REQUIRE(program_counter.value() == 0x0001); // NOLINT
    REQUIRE(cycles.value() == 16); // NOLINT
    
    // Execute PUSH BC
    cycles = cpu::push_rp(registers, memory, program_counter);
    REQUIRE(registers.get_sp().value() == 0xFFFA); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFFA}) == 0x78); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFFB}) == 0x56); // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 16); // NOLINT
    
    // Execute PUSH DE
    cycles = cpu::push_rp(registers, memory, program_counter);
    REQUIRE(registers.get_sp().value() == 0xFFF8); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFF8}) == 0xBC); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFF9}) == 0x9A); // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // NOLINT
    REQUIRE(cycles.value() == 16); // NOLINT
    
    // Execute PUSH HL
    cycles = cpu::push_rp(registers, memory, program_counter);
    REQUIRE(registers.get_sp().value() == 0xFFF6); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFF6}) == 0xF0); // NOLINT
    REQUIRE(memory.read(core::Address{0xFFF7}) == 0xDE); // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 16); // NOLINT
    
    // Now pop everything back
    
    // Execute POP HL (should get HL's value)
    registers.set_hl(core::RegisterPair{0x0000}); // Clear HL first // NOLINT
    cycles = cpu::pop_rp(registers, memory, program_counter);
    REQUIRE(registers.get_hl().value() == 0xDEF0); // NOLINT
    REQUIRE(registers.get_sp().value() == 0xFFF8); // NOLINT
    REQUIRE(program_counter.value() == 0x0005); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Execute POP DE (should get DE's value)
    registers.set_de(core::RegisterPair{0x0000}); // Clear DE first // NOLINT
    cycles = cpu::pop_rp(registers, memory, program_counter);
    REQUIRE(registers.get_de().value() == 0x9ABC); // NOLINT
    REQUIRE(registers.get_sp().value() == 0xFFFA); // NOLINT
    REQUIRE(program_counter.value() == 0x0006); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Execute POP BC (should get BC's value)
    registers.set_bc(core::RegisterPair{0x0000}); // Clear BC first // NOLINT
    cycles = cpu::pop_rp(registers, memory, program_counter);
    REQUIRE(registers.get_bc().value() == 0x5678); // NOLINT
    REQUIRE(registers.get_sp().value() == 0xFFFC); // NOLINT
    REQUIRE(program_counter.value() == 0x0007); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Execute POP AF (should get AF's value)
    registers.set_af(core::RegisterPair{0x0000}); // Clear AF first // NOLINT
    cycles = cpu::pop_rp(registers, memory, program_counter);
    REQUIRE(registers.get_af().value() == 0x1230); // Note: lower 4 bits of F are masked to 0 // NOLINT
    REQUIRE(registers.get_sp().value() == 0xFFFE); // NOLINT
    REQUIRE(program_counter.value() == 0x0008); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
  }
}

TEST_CASE("Memory-specific load instructions", "[cpu][load][memory]") { // NOLINT
  using namespace cycle_perfect_gameboy; // NOLINT
  
  MockMemory memory;
  cpu::Registers registers;
  
  SECTION("LD A,(BC), LD A,(DE), LD A,(HL+), LD A,(HL-) work correctly") { // NOLINT
    // Setup registers and memory
    registers.set_bc(core::RegisterPair{0x1000}); // NOLINT
    registers.set_de(core::RegisterPair{0x2000}); // NOLINT
    registers.set_hl(core::RegisterPair{0x3000}); // NOLINT
    
    // Set values in memory
    memory.write(core::Address{0x1000}, 0x42); // at BC // NOLINT
    memory.write(core::Address{0x2000}, 0x84); // at DE // NOLINT
    memory.write(core::Address{0x3000}, 0xA5); // at HL // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x0A}); // LD A,(BC) // NOLINT
    memory.set_memory(core::Address{0x0001}, {0x1A}); // LD A,(DE) // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x2A}); // LD A,(HL+) // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x3A}); // LD A,(HL-) // NOLINT
    
    // Execute LD A,(BC)
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_a_mem(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_a() == 0x42); // NOLINT
    REQUIRE(registers.get_bc().value() == 0x1000); // BC unchanged // NOLINT
    REQUIRE(program_counter.value() == 0x0001); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD A,(DE)
    cycles = cpu::ld_a_mem(registers, memory, program_counter);
    REQUIRE(registers.get_a() == 0x84); // NOLINT
    REQUIRE(registers.get_de().value() == 0x2000); // DE unchanged // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD A,(HL+)
    cycles = cpu::ld_a_mem(registers, memory, program_counter);
    REQUIRE(registers.get_a() == 0xA5); // NOLINT
    REQUIRE(registers.get_hl().value() == 0x3001); // HL incremented // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD A,(HL-)
    // First set a new value at the current HL
    memory.write(core::Address{0x3001}, 0xC7); // NOLINT
    
    cycles = cpu::ld_a_mem(registers, memory, program_counter);
    REQUIRE(registers.get_a() == 0xC7); // NOLINT
    REQUIRE(registers.get_hl().value() == 0x3000); // HL decremented // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
  
  SECTION("LD (BC),A, LD (DE),A, LD (HL+),A, LD (HL-),A work correctly") { // NOLINT
    // Setup registers
    registers.set_a(0x42); // NOLINT
    registers.set_bc(core::RegisterPair{0x1000}); // NOLINT
    registers.set_de(core::RegisterPair{0x2000}); // NOLINT
    registers.set_hl(core::RegisterPair{0x3000}); // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x02}); // LD (BC),A // NOLINT
    memory.set_memory(core::Address{0x0001}, {0x12}); // LD (DE),A // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x22}); // LD (HL+),A // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x32}); // LD (HL-),A // NOLINT
    
    // Change A value for each test to make it easier to verify
    registers.set_a(0x42); // NOLINT
    
    // Execute LD (BC),A
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_mem_a(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(memory.read(core::Address{0x1000}) == 0x42); // NOLINT
    REQUIRE(registers.get_bc().value() == 0x1000); // BC unchanged // NOLINT
    REQUIRE(program_counter.value() == 0x0001); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Update A and execute LD (DE),A
    registers.set_a(0x84); // NOLINT
    cycles = cpu::ld_mem_a(registers, memory, program_counter);
    REQUIRE(memory.read(core::Address{0x2000}) == 0x84); // NOLINT
    REQUIRE(registers.get_de().value() == 0x2000); // DE unchanged // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Update A and execute LD (HL+),A
    registers.set_a(0xA5); // NOLINT
    cycles = cpu::ld_mem_a(registers, memory, program_counter);
    REQUIRE(memory.read(core::Address{0x3000}) == 0xA5); // NOLINT
    REQUIRE(registers.get_hl().value() == 0x3001); // HL incremented // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Update A and execute LD (HL-),A
    registers.set_a(0xC7); // NOLINT
    cycles = cpu::ld_mem_a(registers, memory, program_counter);
    REQUIRE(memory.read(core::Address{0x3001}) == 0xC7); // NOLINT
    REQUIRE(registers.get_hl().value() == 0x3000); // HL decremented // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
  
  SECTION("I/O port instructions LD ($FF00+n),A and LD A,($FF00+n) work correctly") { // NOLINT
    // Setup registers
    registers.set_a(0x42); // NOLINT
    registers.set_c(0x10); // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0xE0, 0x80}); // LD ($FF00+n),A // NOLINT
    memory.set_memory(core::Address{0x0002}, {0xF0, 0x80}); // LD A,($FF00+n) // NOLINT
    memory.set_memory(core::Address{0x0004}, {0xE2}); // LD ($FF00+C),A // NOLINT
    memory.set_memory(core::Address{0x0005}, {0xF2}); // LD A,($FF00+C) // NOLINT
    
    // Execute LD ($FF00+n),A
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_io_a(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(memory.read(core::Address{0xFF80}) == 0x42); // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Change A and execute LD A,($FF00+n)
    registers.set_a(0x00); // NOLINT
    cycles = cpu::ld_io_a(registers, memory, program_counter);
    REQUIRE(registers.get_a() == 0x42); // NOLINT
    REQUIRE(program_counter.value() == 0x0004); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Change value at I/O port address and A, then execute LD ($FF00+C),A
    registers.set_a(0x84); // NOLINT
    cycles = cpu::ld_io_a(registers, memory, program_counter);
    REQUIRE(memory.read(core::Address{0xFF10}) == 0x84); // NOLINT
    REQUIRE(program_counter.value() == 0x0005); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    
    // Change A and execute LD A,($FF00+C)
    registers.set_a(0x00); // NOLINT
    cycles = cpu::ld_io_a(registers, memory, program_counter);
    REQUIRE(registers.get_a() == 0x84); // NOLINT
    REQUIRE(program_counter.value() == 0x0006); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
  }

  SECTION("LD HL,SP+n works correctly with positive offset") { // NOLINT
    // Setup SP
    registers.set_sp(core::RegisterPair{0x1000}); // NOLINT
    
    // Set up memory opcode with positive offset
    memory.set_memory(core::Address{0x0000}, {0xF8, 0x02}); // LD HL,SP+2 // NOLINT
    
    // Execute LD HL,SP+n
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_hl_sp_n(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_hl().value() == 0x1002); // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    REQUIRE(registers.get_zero_flag() == false);
    REQUIRE(registers.get_subtract_flag() == false);
    REQUIRE(registers.get_half_carry_flag() == false);
    REQUIRE(registers.get_carry_flag() == false);
  }
  
  SECTION("LD HL,SP+n works correctly with negative offset") { // NOLINT
    // Setup SP
    registers.set_sp(core::RegisterPair{0x1000}); // NOLINT
    
    // Set up memory opcode with negative offset
    memory.set_memory(core::Address{0x0000}, {0xF8, 0xFE}); // LD HL,SP-2 // NOLINT
    
    // Execute LD HL,SP+n
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_hl_sp_n(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_hl().value() == 0x0FFE); // NOLINT
    REQUIRE(program_counter.value() == 0x0002); // NOLINT
    REQUIRE(cycles.value() == 12); // NOLINT
    REQUIRE(registers.get_zero_flag() == false);
    REQUIRE(registers.get_subtract_flag() == false);
    // Half-carry and carry flags depend on implementation details
  }

  SECTION("LD SP,HL works correctly") { // NOLINT
    // Setup HL
    registers.set_hl(core::RegisterPair{0x1234}); // NOLINT
    
    // Set up memory opcode
    memory.set_memory(core::Address{0x0000}, {0xF9}); // LD SP,HL // NOLINT
    
    // Execute LD SP,HL
    auto program_counter = core::Address{0x0000}; // NOLINT
    auto cycles = cpu::ld_sp_hl(registers, memory, program_counter);
    
    // Verify results
    REQUIRE(registers.get_sp().value() == 0x1234); // NOLINT
    REQUIRE(program_counter.value() == 0x0001); // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
}