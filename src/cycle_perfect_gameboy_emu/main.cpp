#include <iostream>
#include <string>
#include <format>
#include <span>
#include <cstdint>
#include <cstddef>

#include <cycle_perfect_gameboy/system/gameboy.hpp>
#include <cycle_perfect_gameboy/cartridge/cartridge_header.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

int main(int argc, char** argv) {
  if (argc < 2) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::cerr << "Usage: " << argv[0] << " <rom_file>\n";
    return 1;
  }
  
  // Use span for safe command-line argument access
  const std::span<char*> args(argv, static_cast<std::size_t>(argc));
  const std::string rom_path = args[1];
  
  // Create GameBoy system
  cycle_perfect_gameboy::system::GameBoy gameboy;
  
  // Load ROM
  if (!gameboy.load_rom(rom_path)) {
    std::cerr << "Failed to load ROM: " << rom_path << "\n";
    return 1;
  }
  
  // Display ROM information
  const auto& cartridge = gameboy.get_cartridge();
  const auto& header = cartridge.get_header();
  
  // ROM size calculation constants
  constexpr std::uint32_t base_rom_size_kb = 32U;
  
  std::cout << std::format("ROM Information for: {}\n", rom_path);
  std::cout << std::format("Title: {}\n", header.get_title());
  std::cout << std::format("Cartridge Type: {:02X}\n", static_cast<int>(header.get_cartridge_type()));
  std::cout << std::format("ROM Size: {:02X} ({} KB)\n", 
                          static_cast<int>(header.get_rom_size()),
                          base_rom_size_kb << static_cast<std::uint32_t>(header.get_rom_size()));
  std::cout << std::format("ROM Banks: {}\n", header.get_rom_banks());
  std::cout << std::format("RAM Size: {:02X}\n", static_cast<int>(header.get_ram_size()));
  std::cout << std::format("RAM Banks: {}\n", header.get_ram_banks());
  std::cout << std::format("Region: {}\n", header.is_japanese() ? "Japanese" : "Non-Japanese");
  
  // Power on the system
  gameboy.power_on();
  
  // Run for some cycles as a demonstration
  constexpr std::uint32_t demo_cycles = 1000U;
  std::cout << "\nEmulation starting...\n";
  gameboy.run_for_cycles(demo_cycles);
  
  // Display CPU state
  const auto& cpu = gameboy.get_cpu();
  const auto& registers = cpu.get_registers();
  
  std::cout << "\nCPU State after execution:\n";
  std::cout << std::format("AF: {:04X}\n", registers.af_.get());
  std::cout << std::format("BC: {:04X}\n", registers.bc_.get());
  std::cout << std::format("DE: {:04X}\n", registers.de_.get());
  std::cout << std::format("HL: {:04X}\n", registers.hl_.get());
  std::cout << std::format("SP: {:04X}\n", registers.sp_.get());
  std::cout << std::format("PC: {:04X}\n", registers.pc_.get());
  std::cout << std::format("Flags: Z={} N={} H={} C={}\n",
                          registers.flag_zero() ? "1" : "0",
                          registers.flag_subtract() ? "1" : "0",
                          registers.flag_half_carry() ? "1" : "0",
                          registers.flag_carry() ? "1" : "0");
  
  return 0;
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)