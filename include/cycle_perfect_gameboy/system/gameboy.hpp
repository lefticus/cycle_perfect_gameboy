#ifndef CYCLE_PERFECT_GAMEBOY_SYSTEM_GAMEBOY_HPP
#define CYCLE_PERFECT_GAMEBOY_SYSTEM_GAMEBOY_HPP

#include <memory>
#include <string>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_map.hpp>
#include <cycle_perfect_gameboy/memory/ram.hpp>
#include <cycle_perfect_gameboy/cpu/cpu.hpp>
#include <cycle_perfect_gameboy/cartridge/rom.hpp>

namespace cycle_perfect_gameboy::system {

class GameBoy {
private:
  // Memory components
  memory::MemoryMap memory_map_{};
  memory::RAM<0x8000> vram_{}; // Video RAM: 8KB (0x8000-0x9FFF)
  memory::RAM<0x2000> wram_{}; // Work RAM: 8KB (0xC000-0xDFFF)
  memory::RAM<0x7F> io_registers_{}; // I/O Registers: 127 bytes (0xFF00-0xFF7F)
  memory::RAM<0x80> hram_{}; // High RAM: 128 bytes (0xFF80-0xFFFF)
  
  // Cartridge ROM
  std::shared_ptr<cartridge::ROM> cartridge_{std::make_shared<cartridge::ROM>()};
  
  // CPU
  cpu::CPU cpu_{memory_map_, memory_map_};
  
  // System state
  bool powered_on_{false};

public:
  GameBoy() {
    initialize_memory_map();
  }
  
  constexpr void initialize_memory_map() {
    // Map ROM area (0x0000-0x7FFF)
    memory_map_.add_region(core::Address{0x0000}, core::Address{0x7FFF}, cartridge_);
    
    // Map VRAM (0x8000-0x9FFF)
    memory_map_.add_region(core::Address{0x8000}, core::Address{0x9FFF}, std::make_shared<memory::RAM<0x8000>>(vram_));
    
    // Map External RAM (0xA000-0xBFFF) - will be handled by MBC implementation
    // For now, we'll use a placeholder
    memory_map_.add_region(core::Address{0xA000}, core::Address{0xBFFF}, std::make_shared<memory::RAM<0x2000>>());
    
    // Map Working RAM (0xC000-0xDFFF)
    memory_map_.add_region(core::Address{0xC000}, core::Address{0xDFFF}, std::make_shared<memory::RAM<0x2000>>(wram_));
    
    // Map Echo RAM (0xE000-0xFDFF) - mirror of 0xC000-0xDDFF
    // We'll use the same WRAM object for Echo RAM
    memory_map_.add_region(core::Address{0xE000}, core::Address{0xFDFF}, std::make_shared<memory::RAM<0x2000>>(wram_));
    
    // Map OAM (0xFE00-0xFE9F) - We'll implement this with PPU later
    memory_map_.add_region(core::Address{0xFE00}, core::Address{0xFE9F}, std::make_shared<memory::RAM<0xA0>>());
    
    // Unmapped (0xFEA0-0xFEFF) - Empty region
    
    // Map I/O Registers (0xFF00-0xFF7F)
    memory_map_.add_region(core::Address{0xFF00}, core::Address{0xFF7F}, std::make_shared<memory::RAM<0x7F>>(io_registers_));
    
    // Map High RAM (0xFF80-0xFFFF)
    memory_map_.add_region(core::Address{0xFF80}, core::Address{0xFFFF}, std::make_shared<memory::RAM<0x80>>(hram_));
  }
  
  [[nodiscard]] bool load_rom(const std::string& rom_path) {
    const bool success = cartridge_->load_from_file(rom_path);
    if (success) {
      // Reset system when loading a new ROM
      reset();
    }
    return success;
  }
  
  void power_on() {
    if (!powered_on_) {
      cpu_.initialize();
      powered_on_ = true;
    }
  }
  
  void reset() {
    // Reset CPU
    cpu_.initialize();
    
    // Reset memory (except cartridge)
    vram_ = memory::RAM<0x8000>{};
    wram_ = memory::RAM<0x2000>{};
    io_registers_ = memory::RAM<0x7F>{};
    hram_ = memory::RAM<0x80>{};
    
    powered_on_ = true;
  }
  
  void run_frame() {
    if (!powered_on_) {
      return;
    }
    
    // For now, we'll just run a fixed number of cycles that approximates one frame
    // Later, we'll synchronize this with the PPU
    constexpr std::uint32_t cycles_per_frame = 70224; // Cycles per frame at 59.7 fps
    cpu_.run(cycles_per_frame);
  }
  
  void run_for_cycles(std::uint32_t cycles) {
    if (!powered_on_) {
      return;
    }
    
    cpu_.run(cycles);
  }
  
  [[nodiscard]] constexpr auto get_cpu() const -> const cpu::CPU& {
    return cpu_;
  }
  
  [[nodiscard]] constexpr auto get_cartridge() const -> const cartridge::ROM& {
    return *cartridge_;
  }
  
  [[nodiscard]] constexpr auto is_powered_on() const -> bool {
    return powered_on_;
  }
};

} // namespace cycle_perfect_gameboy::system

#endif // CYCLE_PERFECT_GAMEBOY_SYSTEM_GAMEBOY_HPP