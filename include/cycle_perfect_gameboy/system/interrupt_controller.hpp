#ifndef CYCLE_PERFECT_GAMEBOY_SYSTEM_INTERRUPT_CONTROLLER_HPP
#define CYCLE_PERFECT_GAMEBOY_SYSTEM_INTERRUPT_CONTROLLER_HPP

#include <cstdint>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::system {

// Interrupt bit positions
constexpr std::uint8_t INTERRUPT_VBLANK = 0;
constexpr std::uint8_t INTERRUPT_LCD_STAT = 1;
constexpr std::uint8_t INTERRUPT_TIMER = 2;
constexpr std::uint8_t INTERRUPT_SERIAL = 3;
constexpr std::uint8_t INTERRUPT_JOYPAD = 4;

// Interrupt vector addresses
constexpr std::uint16_t VBLANK_VECTOR = 0x0040;
constexpr std::uint16_t LCD_STAT_VECTOR = 0x0048;
constexpr std::uint16_t TIMER_VECTOR = 0x0050;
constexpr std::uint16_t SERIAL_VECTOR = 0x0058;
constexpr std::uint16_t JOYPAD_VECTOR = 0x0060;

// Memory-mapped I/O registers
constexpr core::Address IF_ADDR{0xFF0F}; // Interrupt Flag
constexpr core::Address IE_ADDR{0xFFFF}; // Interrupt Enable

class InterruptController {
private:
  memory::MemoryInterface& memory_;
  
  // Internal state
  std::uint8_t if_{0}; // Interrupt Flag
  std::uint8_t ie_{0}; // Interrupt Enable
  
  // Read registers from memory
  void read_registers() {
    if_ = memory_.read(IF_ADDR);
    ie_ = memory_.read(IE_ADDR);
  }
  
  // Write registers to memory
  void write_registers() {
    memory_.write(IF_ADDR, if_);
    memory_.write(IE_ADDR, ie_);
  }

public:
  explicit InterruptController(memory::MemoryInterface& memory) : memory_(memory) {}
  
  void initialize() {
    // Initialize registers with default values
    if_ = 0xE1; // Unused bits are set to 1
    ie_ = 0x00;
    
    write_registers();
  }
  
  // Request an interrupt by setting its flag
  void request_interrupt(std::uint8_t interrupt) {
    read_registers();
    
    // Set the specified bit in IF
    if_ |= static_cast<std::uint8_t>(1U << interrupt);
    
    write_registers();
  }
  
  // Clear a specific interrupt flag
  void clear_interrupt(std::uint8_t interrupt) {
    read_registers();
    
    // Clear the specified bit in IF
    if_ &= static_cast<std::uint8_t>(~(1U << interrupt));
    
    write_registers();
  }
  
  // Enable a specific interrupt
  void enable_interrupt(std::uint8_t interrupt) {
    read_registers();
    
    // Set the specified bit in IE
    ie_ |= static_cast<std::uint8_t>(1U << interrupt);
    
    write_registers();
  }
  
  // Disable a specific interrupt
  void disable_interrupt(std::uint8_t interrupt) {
    read_registers();
    
    // Clear the specified bit in IE
    ie_ &= static_cast<std::uint8_t>(~(1U << interrupt));
    
    write_registers();
  }
  
  // Check if any interrupts are pending and enabled
  [[nodiscard]] auto interrupts_pending() -> bool {
    read_registers();
    
    // Interrupts are pending if any bits are set in both IF and IE
    return (if_ & ie_ & 0x1F) != 0;
  }
  
  // Get the vector address for the highest priority pending interrupt
  // Returns 0 if no interrupts are pending
  [[nodiscard]] auto get_interrupt_vector() -> std::uint16_t {
    read_registers();
    
    // Calculate which interrupts are both pending and enabled
    const std::uint8_t pending = static_cast<std::uint8_t>(if_ & ie_ & 0x1FU);
    
    if (pending == 0) {
      return 0;
    }
    
    // Check each interrupt in priority order (VBlank has highest priority)
    if ((pending & static_cast<std::uint8_t>(1U << INTERRUPT_VBLANK)) != 0) {
      clear_interrupt(INTERRUPT_VBLANK);
      return VBLANK_VECTOR;
    }
    
    if ((pending & static_cast<std::uint8_t>(1U << INTERRUPT_LCD_STAT)) != 0) {
      clear_interrupt(INTERRUPT_LCD_STAT);
      return LCD_STAT_VECTOR;
    }
    
    if ((pending & static_cast<std::uint8_t>(1U << INTERRUPT_TIMER)) != 0) {
      clear_interrupt(INTERRUPT_TIMER);
      return TIMER_VECTOR;
    }
    
    if ((pending & static_cast<std::uint8_t>(1U << INTERRUPT_SERIAL)) != 0) {
      clear_interrupt(INTERRUPT_SERIAL);
      return SERIAL_VECTOR;
    }
    
    if ((pending & static_cast<std::uint8_t>(1U << INTERRUPT_JOYPAD)) != 0) {
      clear_interrupt(INTERRUPT_JOYPAD);
      return JOYPAD_VECTOR;
    }
    
    return 0; // Should never reach here if pending is non-zero
  }
};

} // namespace cycle_perfect_gameboy::system

#endif // CYCLE_PERFECT_GAMEBOY_SYSTEM_INTERRUPT_CONTROLLER_HPP