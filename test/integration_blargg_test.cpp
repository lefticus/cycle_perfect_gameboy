#include <catch2/catch_test_macros.hpp>

#include "cycle_perfect_gameboy/cartridge/rom.hpp"
#include "cycle_perfect_gameboy/cpu/cpu.hpp"
#include "cycle_perfect_gameboy/system/gameboy.hpp"
#include "cycle_perfect_gameboy/core/types.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
namespace cpgb = cycle_perfect_gameboy;

// Helper function to read serial output from GameBoy emulator
std::string read_serial_output(cpgb::system::GameBoy& gb, std::size_t max_cycles = 10'000'000) {
  std::string output;
  
  // Run the emulator for a maximum number of cycles
  for (std::size_t i = 0; i < max_cycles; ++i) {
    // Execute one machine cycle
    gb.step();
    
    // Check if there's data on the serial port
    if (gb.serial_data_available()) {
      output += static_cast<char>(gb.read_serial_data());
      
      // Check for test completion message
      if (output.find("Passed") != std::string::npos || 
          output.find("Failed") != std::string::npos) {
        break;
      }
    }
  }
  
  return output;
}

TEST_CASE("Blargg's CPU instruction tests - individual ROMs", "[integration][blargg]") {
  const std::vector<std::string> test_names = {
    "01-special.gb",
    "02-interrupts.gb",
    "03-op sp,hl.gb",
    "04-op r,imm.gb",
    "05-op rp.gb",
    "06-ld r,r.gb",
    "07-jr,jp,call,ret,rst.gb",
    "08-misc instrs.gb",
    "09-op r,r.gb",
    "10-bit ops.gb",
    "11-op a,(hl).gb"
  };
  
  const fs::path test_dir = fs::path("/home/jason/cycle_perfect_gameboy/external/blargg_tests/individual");
  
  REQUIRE(fs::exists(test_dir));
  
  for (const auto& test_name : test_names) {
    SECTION(test_name) {
      const fs::path test_rom_path = test_dir / test_name;
      
      REQUIRE(fs::exists(test_rom_path));
      
      // Load the ROM
      auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
      REQUIRE(rom);
      
      // Create GameBoy system with the test ROM
      cpgb::system::GameBoy gameboy(std::move(*rom));
      
      // Initialize the system
      gameboy.reset();
      
      // Run the test and capture serial output
      std::string test_output = read_serial_output(gameboy);
      
      // Output test results for debugging
      INFO("Test output: " << test_output);
      
      // Check for passing tests
      REQUIRE(test_output.find("Failed") == std::string::npos);
      REQUIRE(test_output.find("Passed") != std::string::npos);
    }
  }
}

TEST_CASE("Blargg's CPU instruction tests - combined ROM", "[integration][blargg]") {
  const fs::path test_rom_path = 
    fs::path("/home/jason/cycle_perfect_gameboy/external/blargg_tests/cpu_instrs.gb");
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  cpgb::system::GameBoy gameboy(std::move(*rom));
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  std::string test_output = read_serial_output(gameboy, 20'000'000); // Use more cycles for combined tests
  
  // Output test results for debugging
  INFO("Test output: " << test_output);
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}