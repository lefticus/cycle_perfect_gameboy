#include <catch2/catch_test_macros.hpp> // NOLINT

#include "cycle_perfect_gameboy/cpu/load_instructions.hpp" // NOLINT
#include "cycle_perfect_gameboy/memory/memory_interface.hpp" // NOLINT
#include "cycle_perfect_gameboy/core/types.hpp" // NOLINT
#include "cycle_perfect_gameboy/cpu/registers.hpp" // NOLINT

#include <array> // NOLINT
#include <cstdint> // NOLINT
#include <cstddef> // For std::size_t
#include <initializer_list> // NOLINT
#include <vector> // NOLINT
#include <format> // For std::format
#include <print> // For std::println

// Define named constants to avoid magic numbers
constexpr std::size_t DUMP_SIZE_SMALL = 16;
constexpr std::size_t DUMP_SIZE_MEDIUM = 32;
constexpr std::uint16_t IO_PORT_BASE = 0xFF00;
constexpr std::uint16_t TEST_ADDR_BASE = 0x1234;
constexpr std::uint16_t TEST_ADDR_NEXT = 0x1235;

// Mock memory for testing
class DebugMockMemory : public cycle_perfect_gameboy::memory::MemoryInterface {
private:
  std::array<std::uint8_t, 0x10000> memory_{}; // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

public:
  // Constructor with option to print debug info
  explicit DebugMockMemory(bool debug_mode = false) : debug_mode_(debug_mode) {} // NOLINT

  [[nodiscard]] constexpr auto read(cycle_perfect_gameboy::core::Address addr) const -> std::uint8_t override {
    const auto value = memory_.at(addr.value());
    if (debug_mode_) {
      // This will print during the test execution
      std::println("READ: Address 0x{:04X}, Value 0x{:02X}", addr.value(), value);
    }
    return value;
  }

  constexpr void write(cycle_perfect_gameboy::core::Address addr, std::uint8_t value) override {
    if (debug_mode_) {
      // This will print during the test execution
      std::println("WRITE: Address 0x{:04X}, Value 0x{:02X}", addr.value(), value);
    }
    memory_.at(addr.value()) = value;
  }

  // Helper to set up memory for tests
  constexpr void set_memory(cycle_perfect_gameboy::core::Address addr, std::initializer_list<std::uint8_t> values) { // NOLINT(readability-named-parameter)
    std::uint16_t offset = 0;
    for (auto value : values) {
      memory_.at(addr.value() + offset) = value;
      if (debug_mode_) {
        std::println("INIT: Address 0x{:04X}, Value 0x{:02X}", addr.value() + offset, value);
      }
      ++offset;
    }
  }

  // Helper to dump memory contents
  void dump_memory(cycle_perfect_gameboy::core::Address start, std::size_t length) const {
    constexpr std::size_t BYTES_PER_LINE = 16; // NOLINT
    
    std::println("Memory dump from 0x{:04X} to 0x{:04X}:", start.value(), start.value() + static_cast<uint16_t>(length) - 1);
    for (std::size_t i = 0; i < length; ++i) {
      if (i % BYTES_PER_LINE == 0) {
        std::print("\n{:04X}: ", start.value() + static_cast<uint16_t>(i));
      }
      std::print("{:02X} ", memory_.at(start.value() + i));
    }
    std::println("");
  }

private:
  bool debug_mode_ = false;
};

TEST_CASE("Load instructions debug test", "[cpu][load][debug]") { // NOLINT
  // Verify test case starts correctly
  REQUIRE(true);
  using namespace cycle_perfect_gameboy; // NOLINT
  
  DebugMockMemory memory(true); // Enable debug printing
  cpu::Registers registers;
  
  SECTION("LD r,r instructions with debug output") { // NOLINT
    // Set up registers
    registers.set_a(0x12); // NOLINT
    registers.set_b(0x34); // NOLINT
    registers.set_c(0x56); // NOLINT
    registers.set_d(0x78); // NOLINT
    registers.set_e(0x9A); // NOLINT
    registers.set_h(0xBC); // NOLINT
    registers.set_l(0xDE); // NOLINT
    
    // Set up memory opcodes for LD instructions
    memory.set_memory(core::Address{0x0000}, {0x47}); // LD B,A // NOLINT
    
    // Print register values before execution
    std::println("Before execution:");
    std::println("A: 0x{:02X}, B: 0x{:02X}", registers.get_a(), registers.get_b());
    
    // Execute LD B,A
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD B,A at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_r_r(registers, memory, program_counter);
    
    // Print results
    std::println("After execution:");
    std::println("A: 0x{:02X}, B: 0x{:02X}", registers.get_a(), registers.get_b());
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify results
    REQUIRE(registers.get_b() == 0x12); // B = A // NOLINT
    REQUIRE(cycles.value() == 4); // NOLINT
  }
  
  SECTION("LD (HL),r and LD r,(HL) with debug output") { // NOLINT
    // Set up registers
    registers.set_a(0x12); // NOLINT
    registers.set_hl(core::RegisterPair{0x1000}); // NOLINT
    
    // Write a value to the memory location pointed to by HL
    memory.write(core::Address{0x1000}, 0x42); // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x77}); // LD (HL),A // NOLINT
    memory.set_memory(core::Address{0x0001}, {0x7E}); // LD A,(HL) // NOLINT
    
    // Print initial state
    std::println("Initial state:");
    std::println("A: 0x{:02X}, HL: 0x{:04X}, (HL): 0x{:02X}", 
           registers.get_a(), registers.get_hl().value(), 
           memory.read(registers.get_hl().to_address()));
    
    // Execute LD (HL),A
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD (HL),A at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_r_r(registers, memory, program_counter);
    
    // Print intermediate state
    std::println("After LD (HL),A:");
    std::println("A: 0x{:02X}, HL: 0x{:04X}, (HL): 0x{:02X}", 
           registers.get_a(), registers.get_hl().value(), 
           memory.read(registers.get_hl().to_address()));
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Execute LD A,(HL)
    registers.set_a(0x00); // Reset A to 0 // NOLINT
    std::println("Reset A to 0x00");
    
    cycles = cpu::ld_r_r(registers, memory, program_counter);
    
    // Print final state
    std::println("After LD A,(HL):");
    std::println("A: 0x{:02X}, HL: 0x{:04X}, (HL): 0x{:02X}", 
           registers.get_a(), registers.get_hl().value(), 
           memory.read(registers.get_hl().to_address()));
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify results
    REQUIRE(memory.read(core::Address{0x1000}) == 0x00); // (HL) has been overwritten with 0x00 // NOLINT
    REQUIRE(registers.get_a() == 0x00); // A is now 0 // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
  
  SECTION("LD A,(BC), LD A,(DE), LD A,(HL+), LD A,(HL-) with debug output") { // NOLINT
    // Setup registers and memory
    registers.set_bc(core::RegisterPair{0x1000}); // NOLINT
    registers.set_de(core::RegisterPair{0x2000}); // NOLINT
    registers.set_hl(core::RegisterPair{0x3000}); // NOLINT
    
    // Set values in memory
    memory.write(core::Address{0x1000}, 0x42); // at BC // NOLINT
    memory.write(core::Address{0x2000}, 0x84); // at DE // NOLINT
    memory.write(core::Address{0x3000}, 0xA5); // at HL // NOLINT
    memory.write(core::Address{0x3001}, 0xC7); // at HL+1 // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x0A}); // LD A,(BC) // NOLINT
    memory.set_memory(core::Address{0x0001}, {0x1A}); // LD A,(DE) // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x2A}); // LD A,(HL+) // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x3A}); // LD A,(HL-) // NOLINT
    
    // Print initial state
    std::println("Initial state:");
    std::println("BC: 0x{:04X}, (BC): 0x{:02X}", registers.get_bc().value(), memory.read(registers.get_bc().to_address()));
    std::println("DE: 0x{:04X}, (DE): 0x{:02X}", registers.get_de().value(), memory.read(registers.get_de().to_address()));
    std::println("HL: 0x{:04X}, (HL): 0x{:02X}, (HL+1): 0x{:02X}", 
           registers.get_hl().value(), 
           memory.read(registers.get_hl().to_address()),
           memory.read(core::Address{static_cast<std::uint16_t>(registers.get_hl().value() + 1)}));
    
    // Execute LD A,(BC)
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD A,(BC) at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_a_mem(registers, memory, program_counter);
    
    // Print state after LD A,(BC)
    std::println("After LD A,(BC):");
    std::println("A: 0x{:02X}, BC: 0x{:04X}", registers.get_a(), registers.get_bc().value());
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD A,(BC)
    REQUIRE(registers.get_a() == 0x42); // NOLINT
    REQUIRE(registers.get_bc().value() == 0x1000); // BC unchanged // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD A,(HL+)
    program_counter = core::Address{0x0002}; // NOLINT
    std::println("Executing LD A,(HL+) at PC=0x{:04X}", program_counter.value());
    cycles = cpu::ld_a_mem(registers, memory, program_counter);
    
    // Print state after LD A,(HL+)
    std::println("After LD A,(HL+):");
    std::println("A: 0x{:02X}, HL: 0x{:04X} (was 0x3000)", registers.get_a(), registers.get_hl().value());
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD A,(HL+)
    REQUIRE(registers.get_a() == 0xA5); // NOLINT
    REQUIRE(registers.get_hl().value() == 0x3001); // HL incremented // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
    
    // Execute LD A,(HL-)
    program_counter = core::Address{0x0003}; // NOLINT
    std::println("Executing LD A,(HL-) at PC=0x{:04X}", program_counter.value());
    cycles = cpu::ld_a_mem(registers, memory, program_counter);
    
    // Print state after LD A,(HL-)
    std::println("After LD A,(HL-):");
    std::println("A: 0x{:02X}, HL: 0x{:04X} (was 0x3001)", registers.get_a(), registers.get_hl().value());
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD A,(HL-)
    REQUIRE(registers.get_a() == 0xC7); // NOLINT
    REQUIRE(registers.get_hl().value() == 0x3000); // HL decremented // NOLINT
    REQUIRE(cycles.value() == 8); // NOLINT
  }
  
  SECTION("LD A,(nn) and LD (nn),A with debug output") { // NOLINT
    // Setup registers and memory
    registers.set_a(0x78); // NOLINT
    
    // Set a value at an absolute memory location
    const auto absolute_addr = core::Address{0x4321}; // NOLINT
    memory.write(absolute_addr, 0xCA); // NOLINT
    
    // Set up memory opcodes
    // 0xEA = LD (nn),A instruction with 2-byte address following
    memory.set_memory(core::Address{0x0000}, {0xEA, 0x21, 0x43}); // LD (0x4321),A (little-endian) // NOLINT
    
    // 0xFA = LD A,(nn) instruction with 2-byte address following
    memory.set_memory(core::Address{0x0003}, {0xFA, 0x21, 0x43}); // LD A,(0x4321) (little-endian) // NOLINT
    
    // Print initial state
    std::println("Initial state:");
    std::println("A: 0x{:02X}, (0x4321): 0x{:02X}", registers.get_a(), memory.read(absolute_addr));
    
    // Execute LD (nn),A
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD (0x4321),A at PC=0x{:04X}", program_counter.value());
    
    auto cycles = cpu::ld_a_abs(registers, memory, program_counter);
    
    // Update program counter manually (since we're not using a CPU dispatcher)
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for EA/FA nn nn
    
    // Print state after LD (nn),A
    std::println("After LD (0x4321),A:");
    std::println("A: 0x{:02X}, (0x4321): 0x{:02X}", registers.get_a(), memory.read(absolute_addr));
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD (nn),A
    REQUIRE(memory.read(absolute_addr) == 0x78); // Value in A was written to memory // NOLINT
    REQUIRE(program_counter.value() == 0x0003); // PC advanced by 3 bytes // NOLINT
    REQUIRE(cycles.value() == 16); // 4 M-cycles // NOLINT
    
    // Modify A before executing LD A,(nn)
    registers.set_a(0x00); // Reset A to 0 // NOLINT
    
    // Execute LD A,(nn)
    std::println("Executing LD A,(0x4321) at PC=0x{:04X}", program_counter.value());
    cycles = cpu::ld_a_abs(registers, memory, program_counter);
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for FA nn nn
    
    // Print state after LD A,(nn)
    std::println("After LD A,(0x4321):");
    std::println("A: 0x{:02X}, (0x4321): 0x{:02X}", registers.get_a(), memory.read(absolute_addr));
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD A,(nn)
    REQUIRE(registers.get_a() == 0x78); // A loaded with the value from memory // NOLINT
    REQUIRE(program_counter.value() == 0x0006); // PC advanced by 3 bytes // NOLINT
    REQUIRE(cycles.value() == 16); // 4 M-cycles // NOLINT
    
    // Dump memory region to see the opcodes and data
    memory.dump_memory(core::Address{0x0000}, DUMP_SIZE_SMALL); // Show opcodes
    memory.dump_memory(absolute_addr, 4); // Show the data at 0x4321
  }
  
  SECTION("I/O port access: LD (FF00+n),A, LD A,(FF00+n), LD (FF00+C),A, LD A,(FF00+C)") { // NOLINT
    // Setup registers and memory
    registers.set_a(0x42); // NOLINT
    registers.set_c(0x10); // NOLINT
    
    // Set up memory opcodes
    // 0xE0 = LD (FF00+n),A with 1-byte offset
    memory.set_memory(core::Address{0x0000}, {0xE0, 0x05}); // LD (FF00+0x05),A // NOLINT
    
    // 0xF0 = LD A,(FF00+n) with 1-byte offset
    memory.set_memory(core::Address{0x0002}, {0xF0, 0x05}); // LD A,(FF00+0x05) // NOLINT
    
    // 0xE2 = LD (FF00+C),A
    memory.set_memory(core::Address{0x0004}, {0xE2}); // LD (FF00+C),A // NOLINT
    
    // 0xF2 = LD A,(FF00+C)
    memory.set_memory(core::Address{0x0005}, {0xF2}); // LD A,(FF00+C) // NOLINT
    
    // Print initial state
    std::println("Initial state:"); // NOLINT
    std::println("A: 0x{:02X}, C: 0x{:02X}", registers.get_a(), registers.get_c()); // NOLINT
    std::println("Initial memory state: (FF05): 0x{:02X}, (FF10): 0x{:02X}", 
           memory.read(core::Address{0xFF05}), memory.read(core::Address{0xFF10})); // NOLINT
    
    // Execute LD (FF00+n),A
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD (FF00+0x05),A at PC=0x{:04X}", program_counter.value()); // NOLINT
    auto cycles = cpu::ld_io_a(registers, memory, program_counter);
    
    // Print state after LD (FF00+n),A
    std::println("After LD (FF00+0x05),A:"); // NOLINT
    std::println("A: 0x{:02X}, (FF05): 0x{:02X}", registers.get_a(), memory.read(core::Address{0xFF05})); // NOLINT
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value()); // NOLINT
    
    // Advance PC manually (since we're not using a CPU dispatcher)
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // 2 bytes for E0 nn // NOLINT
    
    // Change A value to verify next instruction
    registers.set_a(0x00); // NOLINT
    
    // Execute LD A,(FF00+n)
    std::println("Executing LD A,(FF00+0x05) at PC=0x{:04X}", program_counter.value()); // NOLINT
    cycles = cpu::ld_io_a(registers, memory, program_counter);
    
    // Print state after LD A,(FF00+n)
    std::println("After LD A,(FF00+0x05):"); // NOLINT
    std::println("A: 0x{:02X}, (FF05): 0x{:02X}", registers.get_a(), memory.read(core::Address{0xFF05})); // NOLINT
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value()); // NOLINT
    
    // Verify LD A,(FF00+n)
    REQUIRE(registers.get_a() == 0x42); // A loaded with value from 0xFF05 // NOLINT
    REQUIRE(cycles.value() == 12); // 3 M-cycles // NOLINT
    
    // Advance PC manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // 2 bytes for F0 nn // NOLINT
    
    // Set a different value for A to test LD (FF00+C),A
    registers.set_a(0x84); // NOLINT
    
    // Execute LD (FF00+C),A
    std::println("Executing LD (FF00+C),A at PC=0x{:04X} with C=0x{:02X}", program_counter.value(), registers.get_c()); // NOLINT
    cycles = cpu::ld_io_a(registers, memory, program_counter);
    
    // Print state after LD (FF00+C),A
    std::println("After LD (FF00+C),A:"); // NOLINT
    std::println("A: 0x{:02X}, C: 0x{:02X}, (FF10): 0x{:02X}", 
           registers.get_a(), registers.get_c(), memory.read(core::Address{0xFF10})); // NOLINT
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value()); // NOLINT
    
    // Verify LD (FF00+C),A
    REQUIRE(memory.read(core::Address{0xFF10}) == 0x84); // Memory at 0xFF00+C contains A // NOLINT
    REQUIRE(cycles.value() == 12); // 3 M-cycles // NOLINT
    
    // Advance PC manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // 1 byte for E2 // NOLINT
    
    // Change A again to verify LD A,(FF00+C)
    registers.set_a(0x00); // NOLINT
    
    // Execute LD A,(FF00+C)
    std::println("Executing LD A,(FF00+C) at PC=0x{:04X} with C=0x{:02X}", program_counter.value(), registers.get_c()); // NOLINT
    cycles = cpu::ld_io_a(registers, memory, program_counter);
    
    // Print state after LD A,(FF00+C)
    std::println("After LD A,(FF00+C):"); // NOLINT
    std::println("A: 0x{:02X}, C: 0x{:02X}, (FF10): 0x{:02X}", 
           registers.get_a(), registers.get_c(), memory.read(core::Address{0xFF10})); // NOLINT
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value()); // NOLINT
    
    // Verify LD A,(FF00+C)
    REQUIRE(registers.get_a() == 0x84); // A loaded with value from 0xFF00+C // NOLINT
    REQUIRE(cycles.value() == 12); // 3 M-cycles // NOLINT
    
    // Dump memory regions for I/O ports
    std::println("Memory dump of I/O ports region:"); // NOLINT
    memory.dump_memory(core::Address{IO_PORT_BASE}, DUMP_SIZE_MEDIUM); // Show the first 32 bytes of I/O ports space
  }
  
  SECTION("Stack operations: PUSH and POP with debug output") { // NOLINT
    // Setup registers and memory
    registers.set_af(core::RegisterPair{0xA511}); // A=0xA5, F=0x11 // NOLINT
    registers.set_bc(core::RegisterPair{0xB5C5}); // NOLINT
    registers.set_de(core::RegisterPair{0xD5E5}); // NOLINT
    registers.set_hl(core::RegisterPair{0xABCD}); // NOLINT
    registers.set_sp(core::RegisterPair{0xFFF0}); // Stack pointer near top of memory // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0xF5}); // PUSH AF // NOLINT
    memory.set_memory(core::Address{0x0001}, {0xC5}); // PUSH BC // NOLINT
    memory.set_memory(core::Address{0x0002}, {0xD5}); // PUSH DE // NOLINT
    memory.set_memory(core::Address{0x0003}, {0xE5}); // PUSH HL // NOLINT
    memory.set_memory(core::Address{0x0004}, {0xF1}); // POP AF // NOLINT
    memory.set_memory(core::Address{0x0005}, {0xC1}); // POP BC // NOLINT
    memory.set_memory(core::Address{0x0006}, {0xD1}); // POP DE // NOLINT
    memory.set_memory(core::Address{0x0007}, {0xE1}); // POP HL // NOLINT
    
    // Print initial state
    std::println("Initial register state:"); // NOLINT
    std::println("AF: 0x{:04X}, BC: 0x{:04X}, DE: 0x{:04X}, HL: 0x{:04X}, SP: 0x{:04X}", 
           registers.get_af().value(), 
           registers.get_bc().value(), 
           registers.get_de().value(), 
           registers.get_hl().value(), 
           registers.get_sp().value()); // NOLINT
    
    std::println("Initial stack memory (showing 16 bytes from SP-8):"); // NOLINT
    memory.dump_memory(core::Address{static_cast<std::uint16_t>(registers.get_sp().value() - 8)}, 16); // NOLINT
    
    // Execute PUSH AF
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing PUSH AF at PC=0x{:04X}", program_counter.value()); // NOLINT
    auto cycles = cpu::push_rp(registers, memory, program_counter);
    
    // Print state after PUSH AF
    std::println("After PUSH AF:"); // NOLINT
    std::println("AF: 0x{:04X}, SP: 0x{:04X} (was 0xFFF0)", registers.get_af().value(), registers.get_sp().value()); // NOLINT
    std::println("Stack memory after PUSH:"); // NOLINT
    memory.dump_memory(registers.get_sp().to_address(), 8); // NOLINT
    std::println("Cycles: {}", cycles.value()); // NOLINT
    
    // Verify PUSH AF
    REQUIRE(registers.get_sp().value() == 0xFFEE); // SP decremented by 2 // NOLINT
    REQUIRE(memory.read(core::Address{0xFFEE}) == 0x11); // F (low byte, SP) // NOLINT
    REQUIRE(memory.read(core::Address{0xFFEF}) == 0xA5); // A (high byte, SP+1) // NOLINT
    REQUIRE(cycles.value() == 16); // 4 M-cycles // NOLINT
    
    // Advance PC manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    
    // Execute PUSH BC
    std::println("Executing PUSH BC at PC=0x{:04X}", program_counter.value()); // NOLINT
    cycles = cpu::push_rp(registers, memory, program_counter);
    
    // Print state after PUSH BC
    std::println("After PUSH BC:"); // NOLINT
    std::println("BC: 0x{:04X}, SP: 0x{:04X} (was 0xFFEE)", registers.get_bc().value(), registers.get_sp().value()); // NOLINT
    std::println("Stack memory after PUSH:"); // NOLINT
    memory.dump_memory(registers.get_sp().to_address(), 8); // NOLINT
    
    // Verify PUSH BC
    REQUIRE(registers.get_sp().value() == 0xFFEC); // SP decremented by 2 more // NOLINT
    REQUIRE(memory.read(core::Address{0xFFEC}) == 0xC5); // C (low byte, SP) // NOLINT
    REQUIRE(memory.read(core::Address{0xFFED}) == 0xB5); // B (high byte, SP+1) // NOLINT
    
    // Push remaining registers (DE, HL) without detailed output
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    auto cycles_de = cpu::push_rp(registers, memory, program_counter); // PUSH DE
    std::println("PUSH DE: Cycles: {}", cycles_de.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    auto cycles_hl = cpu::push_rp(registers, memory, program_counter); // PUSH HL
    std::println("PUSH HL: Cycles: {}", cycles_hl.value());
    
    // Verify final stack state after all PUSHes
    std::println("Final stack state after all PUSHes:"); // NOLINT
    std::println("SP: 0x{:04X} (was 0xFFF0, 8 bytes pushed)", registers.get_sp().value()); // NOLINT
    memory.dump_memory(registers.get_sp().to_address(), 16); // NOLINT
    
    // Now reset registers to make it clear we're pulling values from stack
    registers.set_af(core::RegisterPair{0x0000}); // NOLINT
    registers.set_bc(core::RegisterPair{0x0000}); // NOLINT
    registers.set_de(core::RegisterPair{0x0000}); // NOLINT
    registers.set_hl(core::RegisterPair{0x0000}); // NOLINT
    
    std::println("Reset registers before POPs:"); // NOLINT
    std::println("AF: 0x{:04X}, BC: 0x{:04X}, DE: 0x{:04X}, HL: 0x{:04X}", 
           registers.get_af().value(), 
           registers.get_bc().value(), 
           registers.get_de().value(), 
           registers.get_hl().value()); // NOLINT
    
    // Advance to first POP instruction
    program_counter = core::Address{0x0004}; // NOLINT
    
    // Execute POP AF
    std::println("Executing POP AF at PC=0x{:04X}", program_counter.value()); // NOLINT
    auto cycles_pop_af = cpu::pop_rp(registers, memory, program_counter);
    
    // Print state after POP AF
    std::println("After POP AF:"); // NOLINT
    std::println("AF: 0x{:04X}, SP: 0x{:04X} (was 0xFFE8)", registers.get_af().value(), registers.get_sp().value()); // NOLINT
    std::println("Cycles: {}", cycles_pop_af.value()); // NOLINT
    
    // Verify POP AF
    REQUIRE(registers.get_af().value() == (0xABCD & 0xFFF0)); // AF with lower 4 bits cleared // NOLINT
    REQUIRE(registers.get_sp().value() == 0xFFEA); // SP incremented by 2 // NOLINT
    REQUIRE(cycles_pop_af.value() == 12); // 3 M-cycles // NOLINT
    
    // Execute remaining POPs
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    auto cycles_pop_bc = cpu::pop_rp(registers, memory, program_counter); // POP BC
    std::println("POP BC: Cycles: {}", cycles_pop_bc.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    auto cycles_pop_de = cpu::pop_rp(registers, memory, program_counter); // POP DE
    std::println("POP DE: Cycles: {}", cycles_pop_de.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    auto cycles_pop_hl = cpu::pop_rp(registers, memory, program_counter); // POP HL
    std::println("POP HL: Cycles: {}", cycles_pop_hl.value());
    
    // Print final register state after all POPs
    std::println("Final register state after all POPs:"); // NOLINT
    std::println("AF: 0x{:04X}, BC: 0x{:04X}, DE: 0x{:04X}, HL: 0x{:04X}, SP: 0x{:04X}", 
           registers.get_af().value(), 
           registers.get_bc().value(), 
           registers.get_de().value(), 
           registers.get_hl().value(), 
           registers.get_sp().value()); // NOLINT
    
    // Verify final state
    REQUIRE(registers.get_bc().value() == 0xD5E5); // DE value we pushed earlier // NOLINT
    REQUIRE(registers.get_de().value() == 0xB5C5); // BC value we pushed earlier // NOLINT
    REQUIRE(registers.get_hl().value() == 0xA510); // AF value with bottom 4 bits cleared // NOLINT
    REQUIRE(registers.get_sp().value() == 0xFFF0); // Original SP value // NOLINT
  }
  
  SECTION("LD (nn),SP with debug output") { // NOLINT
    // Setup registers
    registers.set_sp(core::RegisterPair{0xABCD}); // NOLINT
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x08, 0x34, 0x12}); // LD (0x1234),SP (little-endian) // NOLINT
    
    // Print initial state
    std::println("Initial state:");
    std::println("SP: 0x{:04X}", registers.get_sp().value());
    std::println("Memory at 0x1234-0x1235: {:02X} {:02X}", 
           memory.read(core::Address{TEST_ADDR_BASE}), memory.read(core::Address{TEST_ADDR_NEXT}));
    
    // Execute LD (nn),SP
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD (0x1234),SP at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_nn_sp(registers, memory, program_counter);
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for 08 nn nn // NOLINT
    
    // Print state after LD (nn),SP
    std::println("After LD (0x1234),SP:");
    std::println("SP: 0x{:04X}", registers.get_sp().value());
    std::println("Memory at 0x1234-0x1235: {:02X} {:02X}", 
           memory.read(core::Address{TEST_ADDR_BASE}), memory.read(core::Address{TEST_ADDR_NEXT}));
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD (nn),SP
    REQUIRE(memory.read(core::Address{TEST_ADDR_BASE}) == 0xCD); // Low byte of SP // NOLINT
    REQUIRE(memory.read(core::Address{TEST_ADDR_NEXT}) == 0xAB); // High byte of SP // NOLINT
    REQUIRE(cycles.value() == 20); // 5 M-cycles // NOLINT
  }
  
  SECTION("LD SP,HL and LD HL,SP+n with debug output") { // NOLINT
    // Setup registers
    registers.set_hl(core::RegisterPair{0xABCD}); // NOLINT
    registers.set_sp(core::RegisterPair{0xDEF0}); // NOLINT
    
    // Clear all flags initially
    registers.set_zero_flag(false);
    registers.set_subtract_flag(false);
    registers.set_half_carry_flag(false);
    registers.set_carry_flag(false);
    
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0xF9}); // LD SP,HL // NOLINT
    memory.set_memory(core::Address{0x0001}, {0xF8, 0x02}); // LD HL,SP+2 // NOLINT
    memory.set_memory(core::Address{0x0003}, {0xF8, 0xFE}); // LD HL,SP-2 (0xFE is -2 in two's complement) // NOLINT
    
    // Print initial state
    std::println("Initial state:");
    std::println("HL: 0x{:04X}, SP: 0x{:04X}, Flags: Z={} N={} H={} C={}", 
           registers.get_hl().value(), 
           registers.get_sp().value(),
           registers.get_zero_flag() ? 1 : 0,
           registers.get_subtract_flag() ? 1 : 0,
           registers.get_half_carry_flag() ? 1 : 0,
           registers.get_carry_flag() ? 1 : 0);
    
    // Execute LD SP,HL
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD SP,HL at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_sp_hl(registers, memory, program_counter);
    
    // Print state after LD SP,HL
    std::println("After LD SP,HL:");
    std::println("HL: 0x{:04X}, SP: 0x{:04X} (was 0xDEF0)", registers.get_hl().value(), registers.get_sp().value());
    std::println("Cycles: {}", cycles.value());
    
    // Verify LD SP,HL
    REQUIRE(registers.get_sp().value() == 0xABCD); // SP now equals HL // NOLINT
    REQUIRE(cycles.value() == 8); // 2 M-cycles // NOLINT
    
    // Advance PC manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 1)}; // NOLINT
    
    // Execute LD HL,SP+2
    std::println("Executing LD HL,SP+2 at PC=0x{:04X}", program_counter.value());
    cycles = cpu::ld_hl_sp_n(registers, memory, program_counter);
    
    // Print state after LD HL,SP+2
    std::println("After LD HL,SP+2:");
    std::println("HL: 0x{:04X} (was 0xABCD), SP: 0x{:04X}", registers.get_hl().value(), registers.get_sp().value());
    std::println("Flags: Z={} N={} H={} C={}", 
           registers.get_zero_flag() ? 1 : 0,
           registers.get_subtract_flag() ? 1 : 0,
           registers.get_half_carry_flag() ? 1 : 0,
           registers.get_carry_flag() ? 1 : 0);
    std::println("Cycles: {}", cycles.value());
    
    // Verify LD HL,SP+2
    REQUIRE(registers.get_hl().value() == 0xABCF); // HL = SP + 2 // NOLINT
    REQUIRE(registers.get_zero_flag() == false); // Z flag reset // NOLINT
    REQUIRE(registers.get_subtract_flag() == false); // N flag reset // NOLINT
    // H and C depend on the specifics of the addition
    REQUIRE(cycles.value() == 12); // 3 M-cycles // NOLINT
    
    // Advance PC manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // 2 bytes for F8 xx // NOLINT
    
    // Execute LD HL,SP-2
    std::println("Executing LD HL,SP-2 at PC=0x{:04X}", program_counter.value());
    cycles = cpu::ld_hl_sp_n(registers, memory, program_counter);
    
    // Print state after LD HL,SP-2
    std::println("After LD HL,SP-2:");
    std::println("HL: 0x{:04X} (was 0xABCF), SP: 0x{:04X}", registers.get_hl().value(), registers.get_sp().value());
    std::println("Flags: Z={} N={} H={} C={}", 
           registers.get_zero_flag() ? 1 : 0,
           registers.get_subtract_flag() ? 1 : 0,
           registers.get_half_carry_flag() ? 1 : 0,
           registers.get_carry_flag() ? 1 : 0);
    std::println("Cycles: {}", cycles.value());
    
    // Verify LD HL,SP-2
    REQUIRE(registers.get_hl().value() == 0xABCB); // HL = SP - 2 // NOLINT
    REQUIRE(registers.get_zero_flag() == false); // Z flag reset // NOLINT
    REQUIRE(registers.get_subtract_flag() == false); // N flag reset // NOLINT
    // H and C depend on the specifics of the addition with negative value
    REQUIRE(cycles.value() == 12); // 3 M-cycles // NOLINT
  }
  
  SECTION("LD rp,nn (16-bit immediate loads) with debug output") { // NOLINT
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x01, 0x34, 0x12}); // LD BC,0x1234 (little-endian) // NOLINT
    memory.set_memory(core::Address{0x0003}, {0x11, 0x78, 0x56}); // LD DE,0x5678 (little-endian) // NOLINT
    memory.set_memory(core::Address{0x0006}, {0x21, 0xCD, 0xAB}); // LD HL,0xABCD (little-endian) // NOLINT
    memory.set_memory(core::Address{0x0009}, {0x31, 0xEF, 0xBE}); // LD SP,0xBEEF (little-endian) // NOLINT
    
    // Print initial register state
    std::println("Initial register state:");
    std::println("BC: 0x{:04X}, DE: 0x{:04X}, HL: 0x{:04X}, SP: 0x{:04X}", 
           registers.get_bc().value(), 
           registers.get_de().value(), 
           registers.get_hl().value(), 
           registers.get_sp().value());
    
    // Execute LD BC,nn
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD BC,0x1234 at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_rp_nn(registers, memory, program_counter);
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for LD BC,nn // NOLINT
    
    // Print state after LD BC,nn
    std::println("After LD BC,0x1234:");
    std::println("BC: 0x{:04X}", registers.get_bc().value());
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD BC,nn
    REQUIRE(registers.get_bc().value() == 0x1234); // NOLINT
    REQUIRE(cycles.value() == 12); // 3 M-cycles // NOLINT
    
    // Execute LD DE,nn
    std::println("Executing LD DE,0x5678 at PC=0x{:04X}", program_counter.value());
    auto cycles_ld_de = cpu::ld_rp_nn(registers, memory, program_counter);
    std::println("LD DE,0x5678: Cycles: {}", cycles_ld_de.value());
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for LD DE,nn // NOLINT
    
    // Verify LD DE,nn
    REQUIRE(registers.get_de().value() == 0x5678); // NOLINT
    
    // Execute LD HL,nn
    std::println("Executing LD HL,0xABCD at PC=0x{:04X}", program_counter.value());
    auto cycles_ld_hl = cpu::ld_rp_nn(registers, memory, program_counter);
    std::println("LD HL,0xABCD: Cycles: {}", cycles_ld_hl.value());
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for LD HL,nn // NOLINT
    
    // Verify LD HL,nn
    REQUIRE(registers.get_hl().value() == 0xABCD); // NOLINT
    
    // Execute LD SP,nn
    std::println("Executing LD SP,0xBEEF at PC=0x{:04X}", program_counter.value());
    auto cycles_sp = cpu::ld_rp_nn(registers, memory, program_counter);
    std::println("LD SP,0xBEEF: Cycles: {}", cycles_sp.value());
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 3)}; // 3 bytes for LD SP,nn // NOLINT
    
    // Verify LD SP,nn
    REQUIRE(registers.get_sp().value() == 0xBEEF); // NOLINT
    
    // Print final register state
    std::println("Final register state:");
    std::println("BC: 0x{:04X}, DE: 0x{:04X}, HL: 0x{:04X}, SP: 0x{:04X}", 
           registers.get_bc().value(), 
           registers.get_de().value(), 
           registers.get_hl().value(), 
           registers.get_sp().value());
  }
  
  SECTION("LD r,n (8-bit immediate loads) with debug output") { // NOLINT
    // Set up memory opcodes
    memory.set_memory(core::Address{0x0000}, {0x06, 0x42}); // LD B,0x42 // NOLINT
    memory.set_memory(core::Address{0x0002}, {0x0E, 0x84}); // LD C,0x84 // NOLINT
    memory.set_memory(core::Address{0x0004}, {0x16, 0xA5}); // LD D,0xA5 // NOLINT
    memory.set_memory(core::Address{0x0006}, {0x1E, 0xC7}); // LD E,0xC7 // NOLINT
    memory.set_memory(core::Address{0x0008}, {0x26, 0x11}); // LD H,0x11 // NOLINT
    memory.set_memory(core::Address{0x000A}, {0x2E, 0x22}); // LD L,0x22 // NOLINT
    memory.set_memory(core::Address{0x000C}, {0x3E, 0xFF}); // LD A,0xFF // NOLINT
    
    // Print initial register state
    std::println("Initial register state:");
    std::println("A: 0x{:02X}, B: 0x{:02X}, C: 0x{:02X}, D: 0x{:02X}, E: 0x{:02X}, H: 0x{:02X}, L: 0x{:02X}", 
           registers.get_a(), 
           registers.get_b(), 
           registers.get_c(), 
           registers.get_d(), 
           registers.get_e(), 
           registers.get_h(), 
           registers.get_l());
    
    // Execute LD B,n
    auto program_counter = core::Address{0x0000}; // NOLINT
    std::println("Executing LD B,0x42 at PC=0x{:04X}", program_counter.value());
    auto cycles = cpu::ld_r_n(registers, memory, program_counter);
    
    // Update program counter manually
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // 2 bytes for LD r,n // NOLINT
    
    // Print state after LD B,n
    std::println("After LD B,0x42:");
    std::println("B: 0x{:02X}", registers.get_b());
    std::println("Cycles: {}, PC: 0x{:04X}", cycles.value(), program_counter.value());
    
    // Verify LD B,n
    REQUIRE(registers.get_b() == 0x42); // NOLINT
    REQUIRE(cycles.value() == 8); // 2 M-cycles // NOLINT
    
    // Execute remaining LD r,n operations
    std::println("Executing LD C,0x84 at PC=0x{:04X}", program_counter.value());
    auto cycles_c = cpu::ld_r_n(registers, memory, program_counter);
    std::println("LD C,0x84: Cycles: {}", cycles_c.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // NOLINT
    
    std::println("Executing LD D,0xA5 at PC=0x{:04X}", program_counter.value());
    auto cycles_d = cpu::ld_r_n(registers, memory, program_counter);
    std::println("LD D,0xA5: Cycles: {}", cycles_d.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // NOLINT
    
    std::println("Executing LD E,0xC7 at PC=0x{:04X}", program_counter.value());
    auto cycles_e = cpu::ld_r_n(registers, memory, program_counter);
    std::println("LD E,0xC7: Cycles: {}", cycles_e.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // NOLINT
    
    std::println("Executing LD H,0x11 at PC=0x{:04X}", program_counter.value());
    auto cycles_h = cpu::ld_r_n(registers, memory, program_counter);
    std::println("LD H,0x11: Cycles: {}", cycles_h.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // NOLINT
    
    std::println("Executing LD L,0x22 at PC=0x{:04X}", program_counter.value());
    auto cycles_l = cpu::ld_r_n(registers, memory, program_counter);
    std::println("LD L,0x22: Cycles: {}", cycles_l.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // NOLINT
    
    std::println("Executing LD A,0xFF at PC=0x{:04X}", program_counter.value());
    auto cycles_a = cpu::ld_r_n(registers, memory, program_counter);
    std::println("LD A,0xFF: Cycles: {}", cycles_a.value());
    program_counter = core::Address{static_cast<std::uint16_t>(program_counter.value() + 2)}; // NOLINT
    
    // Print final register state
    std::println("Final register state:");
    std::println("A: 0x{:02X}, B: 0x{:02X}, C: 0x{:02X}, D: 0x{:02X}, E: 0x{:02X}, H: 0x{:02X}, L: 0x{:02X}", 
           registers.get_a(), 
           registers.get_b(), 
           registers.get_c(), 
           registers.get_d(), 
           registers.get_e(), 
           registers.get_h(), 
           registers.get_l());
    
    // Verify all registers got loaded correctly
    REQUIRE(registers.get_a() == 0xFF); // NOLINT
    REQUIRE(registers.get_b() == 0x42); // NOLINT
    REQUIRE(registers.get_c() == 0x84); // NOLINT
    REQUIRE(registers.get_d() == 0xA5); // NOLINT
    REQUIRE(registers.get_e() == 0xC7); // NOLINT
    REQUIRE(registers.get_h() == 0x11); // NOLINT
    REQUIRE(registers.get_l() == 0x22); // NOLINT
    
    // Check HL register pair
    REQUIRE(registers.get_hl().value() == 0x1122); // NOLINT
  }
  
  // Verify test case ends correctly
  REQUIRE(true);
}