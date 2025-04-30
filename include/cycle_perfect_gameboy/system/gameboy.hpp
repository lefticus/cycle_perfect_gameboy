#ifndef CYCLE_PERFECT_GAMEBOY_SYSTEM_GAMEBOY_HPP
#define CYCLE_PERFECT_GAMEBOY_SYSTEM_GAMEBOY_HPP

#include <memory>
#include <queue>
#include <string>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_map.hpp>
#include <cycle_perfect_gameboy/memory/ram.hpp>
#include <cycle_perfect_gameboy/cpu/cpu.hpp>
#include <cycle_perfect_gameboy/cartridge/rom.hpp>
#include <cycle_perfect_gameboy/system/timer.hpp>
#include <cycle_perfect_gameboy/system/interrupt_controller.hpp>

namespace cycle_perfect_gameboy::system {

// Serial port addresses
constexpr core::Address SB_ADDR{0xFF01}; // Serial transfer data
constexpr core::Address SC_ADDR{0xFF02}; // Serial transfer control

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
  
  // System components
  std::unique_ptr<InterruptController> interrupt_controller_;
  std::unique_ptr<Timer> timer_;
  
  // CPU
  cpu::CPU cpu_{memory_map_, memory_map_};
  
  // System state
  bool powered_on_{false};
  
  // Serial port
  std::queue<std::uint8_t> serial_buffer_{};

public:
  GameBoy() {
    initialize_memory_map();
    
    // Initialize system components
    interrupt_controller_ = std::make_unique<InterruptController>(memory_map_);
    timer_ = std::make_unique<Timer>(memory_map_);
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
      // Initialize all components
      cpu_.initialize();
      interrupt_controller_->initialize();
      timer_->initialize();
      
      powered_on_ = true;
    }
  }
  
  void reset() {
    // Reset CPU
    cpu_.initialize();
    
    // Reset system components
    interrupt_controller_->initialize();
    timer_->initialize();
    
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
    run_for_cycles(cycles_per_frame);
  }
  
  void run_for_cycles(std::uint32_t total_cycles) {
    if (!powered_on_) {
      return;
    }
    
    std::uint32_t cycles_executed = 0;
    while (cycles_executed < total_cycles) {
      // Execute a single instruction
      const auto cycles = cpu_.execute_instruction();
      const auto cycle_count = cycles.value();
      
      // Update timer
      timer_->update(cycles);
      
      // Additional components like PPU would be updated here
      
      cycles_executed += cycle_count;
    }
  }
  
  // Special memory access handlers for memory-mapped I/O
  void handle_memory_write(core::Address addr, std::uint8_t value) {
    const auto addr_value = addr.value();
    
    // Handle divider register reset
    if (addr_value == DIV_ADDR.value()) {
      timer_->reset_div();
    }
    
    // Handle timer control updates
    else if (addr_value == TAC_ADDR.value()) {
      const auto old_tac = memory_map_.read(TAC_ADDR);
      timer_->tac_updated(old_tac, value);
    }
    
    // Other memory-mapped hardware would be handled here
  }
  
  [[nodiscard]] constexpr auto get_cpu() const -> const cpu::CPU& {
    return cpu_;
  }
  
  [[nodiscard]] constexpr auto get_cartridge() const -> const cartridge::ROM& {
    return *cartridge_;
  }
  
  [[nodiscard]] constexpr auto get_timer() const -> const Timer& {
    return *timer_;
  }
  
  [[nodiscard]] constexpr auto get_interrupt_controller() const -> const InterruptController& {
    return *interrupt_controller_;
  }
  
  [[nodiscard]] constexpr auto is_powered_on() const -> bool {
    return powered_on_;
  }
  
  // Step a single machine cycle
  auto step() -> core::Cycles {
    if (!powered_on_) {
      return core::Cycles{4}; // Return a default cycle count
    }
    
    // Execute a single instruction
    const auto cycles = cpu_.execute_instruction();
    
    // Update timer
    timer_->update(cycles);
    
    // Handle serial transfer
    handle_serial_transfer();
    
    return cycles;
  }
  
  // Serial port handling
  void handle_serial_transfer() {
    const auto sc_value = memory_map_.read(SC_ADDR);
    
    // Check if a transfer has been requested (bit 7 set)
    if (sc_value & 0x80) {
      // Read serial data and add to buffer
      const auto data = memory_map_.read(SB_ADDR);
      serial_buffer_.push(data);
      
      // Reset transfer flag to indicate completion
      memory_map_.write(SC_ADDR, sc_value & ~0x80);
    }
  }
  
  // Check if serial data is available
  [[nodiscard]] bool serial_data_available() const {
    return !serial_buffer_.empty();
  }
  
  // Read serial data from buffer
  std::uint8_t read_serial_data() {
    if (serial_buffer_.empty()) {
      return 0;
    }
    
    const auto data = serial_buffer_.front();
    serial_buffer_.pop();
    return data;
  }
  
  // Constructor that takes a ROM
  explicit GameBoy(cartridge::ROM rom) : cartridge_{std::make_shared<cartridge::ROM>(std::move(rom))} {
    initialize_memory_map();
    
    // Initialize system components
    interrupt_controller_ = std::make_unique<InterruptController>(memory_map_);
    timer_ = std::make_unique<Timer>(memory_map_);
  }
};

} // namespace cycle_perfect_gameboy::system

#endif // CYCLE_PERFECT_GAMEBOY_SYSTEM_GAMEBOY_HPP