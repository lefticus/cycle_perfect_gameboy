#include <catch2/catch_test_macros.hpp> // NOLINT

// NOLINT(*) for all includes below
#include "cycle_perfect_gameboy/cartridge/rom.hpp" // NOLINT
#include "cycle_perfect_gameboy/system/gameboy.hpp" // NOLINT
#include "cycle_perfect_gameboy/debug/logger.hpp" // NOLINT

#include <cstddef> // For std::size_t
#include <filesystem> // NOLINT
#include <optional> // For std::optional
#include <string> // NOLINT
#include <utility> // For std::move
#include <vector> // NOLINT

namespace fs = std::filesystem;
namespace cpgb = cycle_perfect_gameboy;

// Initialize logger for debugging
void setup_logger(bool enable_debug, const std::string& test_name = "") {
  if (enable_debug) {
    cpgb::debug::Logger::set_level(cpgb::debug::LogLevel::TRACE);
    if (!test_name.empty()) {
      const std::string log_file = "blargg_test_" + test_name + ".log";
      cpgb::debug::Logger::set_log_file(log_file);
      cpgb::debug::Logger::set_console_output(false);
    }
  } else {
    cpgb::debug::Logger::set_level(cpgb::debug::LogLevel::NONE);
  }
}

// Helper function to read serial output from GameBoy emulator
std::string read_serial_output(cpgb::system::GameBoy& gameboy, std::size_t max_cycles = 1'000'000) { // NOLINT(*) for parameter and magic number
  std::string output;
  
  // Constants for logging
  constexpr std::size_t LOG_INTERVAL = 100'000; // NOLINT
  
  // Run the emulator for a maximum number of cycles
  for (std::size_t i = 0; i < max_cycles; ++i) {
    // Execute one machine cycle
    gameboy.step();
    
    // Log progress periodically
    if (i % LOG_INTERVAL == 0) {
      cpgb::debug::Logger::info("Executed " + std::to_string(i) + " cycles");
    }
    
    // Check if there's data on the serial port
    if (gameboy.serial_data_available()) {
      const char data = static_cast<char>(gameboy.read_serial_data());
      output += data;
      cpgb::debug::Logger::debug("Serial output: " + std::string(1, data));
      
      // Check for test completion message
      if (output.find("Passed") != std::string::npos || 
          output.find("Failed") != std::string::npos) {
        break;
      }
    }
  }
  
  return output;
}

// Define test directory path once
const fs::path blargg_test_dir = fs::path("/home/jason/cycle_perfect_gameboy/external/blargg_tests/individual"); // NOLINT

TEST_CASE("Blargg test 01-special", "[integration][blargg]") { // NOLINT
  const std::string test_name = "01-special.gb";
  
  REQUIRE(fs::exists(blargg_test_dir));
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, test_name);
  
  const fs::path test_rom_path = blargg_test_dir / test_name;
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting test: " + test_name);
  const std::string test_output = read_serial_output(gameboy);
  cpgb::debug::Logger::info("Test complete: " + test_name);
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}

TEST_CASE("Blargg test 02-interrupts", "[integration][blargg]") { // NOLINT
  const std::string test_name = "02-interrupts.gb";
  
  REQUIRE(fs::exists(blargg_test_dir));
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, test_name);
  
  const fs::path test_rom_path = blargg_test_dir / test_name;
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting test: " + test_name);
  const std::string test_output = read_serial_output(gameboy);
  cpgb::debug::Logger::info("Test complete: " + test_name);
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}

TEST_CASE("Blargg test 03-op sp,hl", "[integration][blargg]") { // NOLINT
  const std::string test_name = "03-op sp,hl.gb";
  
  REQUIRE(fs::exists(blargg_test_dir));
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, test_name);
  
  const fs::path test_rom_path = blargg_test_dir / test_name;
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting test: " + test_name);
  const std::string test_output = read_serial_output(gameboy);
  cpgb::debug::Logger::info("Test complete: " + test_name);
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}

TEST_CASE("Blargg test 04-op r,imm", "[integration][blargg]") { // NOLINT
  const std::string test_name = "04-op r,imm.gb";
  
  REQUIRE(fs::exists(blargg_test_dir));
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, test_name);
  
  const fs::path test_rom_path = blargg_test_dir / test_name;
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting test: " + test_name);
  const std::string test_output = read_serial_output(gameboy);
  cpgb::debug::Logger::info("Test complete: " + test_name);
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}

TEST_CASE("Blargg test 05-op rp", "[integration][blargg]") { // NOLINT
  const std::string test_name = "05-op rp.gb";
  
  REQUIRE(fs::exists(blargg_test_dir));
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, test_name);
  
  const fs::path test_rom_path = blargg_test_dir / test_name;
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting test: " + test_name);
  const std::string test_output = read_serial_output(gameboy);
  cpgb::debug::Logger::info("Test complete: " + test_name);
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}

TEST_CASE("Blargg test 06-ld r,r", "[integration][blargg]") { // NOLINT
  const std::string test_name = "06-ld r,r.gb";
  
  REQUIRE(fs::exists(blargg_test_dir));
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, test_name);
  
  const fs::path test_rom_path = blargg_test_dir / test_name;
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting test: " + test_name);
  const std::string test_output = read_serial_output(gameboy);
  cpgb::debug::Logger::info("Test complete: " + test_name);
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}

TEST_CASE("Blargg's CPU instruction tests - combined ROM", "[integration][blargg]") { // NOLINT
  const fs::path test_rom_path = 
    fs::path("/home/jason/cycle_perfect_gameboy/external/blargg_tests/cpu_instrs.gb"); // NOLINT
  
  // Setup logging for this test
  const bool enable_logging = true; // Set to false for normal test runs
  setup_logger(enable_logging, "combined_tests");
  
  REQUIRE(fs::exists(test_rom_path));
  
  // Load the ROM
  auto rom = cpgb::cartridge::ROM::from_file(test_rom_path);
  REQUIRE(rom);
  
  // Create GameBoy system with the test ROM
  // Safe to use direct access because we check if rom is valid with REQUIRE(rom) above
  cpgb::system::GameBoy gameboy(std::move(*rom)); // NOLINT(bugprone-unchecked-optional-access)
  
  // Initialize the system
  gameboy.reset();
  
  // Run the test and capture serial output
  cpgb::debug::Logger::info("Starting combined tests");
  const std::string test_output = read_serial_output(gameboy, 20'000'000); // NOLINT - Use more cycles for combined tests
  cpgb::debug::Logger::info("Combined tests complete");
  cpgb::debug::Logger::info("Output: " + test_output);
  
  // Close log file
  cpgb::debug::Logger::close_log_file();
  
  // Output test results for debugging
  INFO("Test output: " << test_output); // NOLINT
  
  // Check for passing tests
  REQUIRE(test_output.find("Failed") == std::string::npos);
  REQUIRE(test_output.find("Passed") != std::string::npos);
}