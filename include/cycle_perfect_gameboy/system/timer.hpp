#ifndef CYCLE_PERFECT_GAMEBOY_SYSTEM_TIMER_HPP
#define CYCLE_PERFECT_GAMEBOY_SYSTEM_TIMER_HPP

#include <cstdint>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::system {

// Addresses of Timer registers
constexpr core::Address DIV_ADDR{0xFF04};  // Divider Register
constexpr core::Address TIMA_ADDR{0xFF05}; // Timer Counter
constexpr core::Address TMA_ADDR{0xFF06};  // Timer Modulo
constexpr core::Address TAC_ADDR{0xFF07};  // Timer Control

// Timer Control (TAC) bit positions
constexpr std::uint8_t TAC_ENABLE = 2;    // Bit 2: Timer enable
constexpr std::uint8_t TAC_CLOCK_SELECT_0 = 0; // Bit 0: Clock select
constexpr std::uint8_t TAC_CLOCK_SELECT_1 = 1; // Bit 1: Clock select

class Timer {
private:
  memory::MemoryInterface& memory_;
  
  // Internal state
  std::uint16_t div_internal_{0};  // 16-bit internal counter - only upper 8 bits are readable
  std::uint16_t prev_div_value_{0}; // For detecting falling edges
  std::uint8_t tima_{0};            // Timer counter
  std::uint8_t tma_{0};             // Timer modulo (reload value)
  std::uint8_t tac_{0};             // Timer control

  // TIMA overflow handling
  bool tima_overflow_{false};
  std::uint16_t overflow_cycles_{0};
  
  // Helper methods
  [[nodiscard]] constexpr auto is_timer_enabled() const noexcept -> bool {
    return (tac_ & (1U << TAC_ENABLE)) != 0;
  }
  
  [[nodiscard]] constexpr auto get_clock_select() const noexcept -> std::uint8_t {
    return tac_ & 0x03U;
  }
  
  [[nodiscard]] constexpr auto get_clock_frequency() const noexcept -> std::uint32_t {
    // Clock frequencies in Hz
    constexpr std::uint32_t frequencies[] = {4096, 262144, 65536, 16384};
    return frequencies[get_clock_select()];
  }
  
  [[nodiscard]] constexpr auto get_input_bit() const noexcept -> std::uint16_t {
    // Bit positions that correspond to different frequencies
    // 0: 4096Hz = bit 9  (1 << 9)
    // 1: 262144Hz = bit 3  (1 << 3)
    // 2: 65536Hz = bit 5  (1 << 5)
    // 3: 16384Hz = bit 7  (1 << 7)
    constexpr std::uint16_t bit_positions[] = {
      1U << 9, 1U << 3, 1U << 5, 1U << 7
    };
    return bit_positions[get_clock_select()];
  }
  
  void read_registers() {
    tima_ = memory_.read(TIMA_ADDR);
    tma_ = memory_.read(TMA_ADDR);
    tac_ = memory_.read(TAC_ADDR);
  }
  
  void write_registers() {
    memory_.write(DIV_ADDR, static_cast<std::uint8_t>(div_internal_ >> 8));
    memory_.write(TIMA_ADDR, tima_);
  }
  
  void request_interrupt() {
    // Set bit 2 (timer interrupt) in the IF register
    const auto interrupt_flag = memory_.read(core::Address{0xFF0F});
    memory_.write(core::Address{0xFF0F}, static_cast<std::uint8_t>(interrupt_flag | (1U << 2)));
  }

public:
  explicit Timer(memory::MemoryInterface& memory) noexcept : memory_(memory) {}
  
  void initialize() {
    div_internal_ = 0;
    prev_div_value_ = 0;
    tima_ = 0;
    tma_ = 0;
    tac_ = 0;
    tima_overflow_ = false;
    overflow_cycles_ = 0;
    
    write_registers();
  }
  
  // Update timer state by the given number of machine cycles
  void update(core::Cycles cycles) {
    read_registers();
    
    const auto cycle_count = cycles.value();
    
    // Handle TIMA overflow with delay
    if (tima_overflow_) {
      overflow_cycles_ += static_cast<std::uint16_t>(cycle_count);
      
      // TIMA overflow takes 4 cycles to trigger an interrupt and reload from TMA
      if (overflow_cycles_ >= 4) {
        tima_ = tma_;
        request_interrupt();
        tima_overflow_ = false;
        overflow_cycles_ = 0;
      }
    }
    
    // Update internal divider register (increments at 16384Hz = CPU freq/256)
    const auto old_div = div_internal_;
    div_internal_ += static_cast<std::uint16_t>(cycle_count * 4); // 4 clock cycles per machine cycle
    
    if (is_timer_enabled()) {
      const auto input_bit = get_input_bit();
      
      // Check if the selected bit has transitioned from 1 to 0 (falling edge)
      const auto old_input = old_div & input_bit;
      const auto new_input = div_internal_ & input_bit;
      
      if (old_input != 0 && new_input == 0) {
        // Increment TIMA on falling edge
        if (tima_ == 0xFF) {
          // TIMA overflow
          tima_ = 0;
          tima_overflow_ = true;
          overflow_cycles_ = 0;
        } else {
          tima_++;
        }
      }
    }
    
    write_registers();
  }
  
  // Reset the DIV register (happens when written to)
  void reset_div() {
    // Read current registers first
    read_registers();
    
    // When DIV is written to, it's reset to 0
    div_internal_ = 0;
    
    // This can trigger a TIMA increment if timer is enabled and a falling edge is detected
    if (is_timer_enabled()) {
      const auto input_bit = get_input_bit();
      
      // Check if the selected bit was 1 (now it's 0 since we reset DIV)
      if ((prev_div_value_ & input_bit) != 0) {
        // Increment TIMA on falling edge
        if (tima_ == 0xFF) {
          // TIMA overflow
          tima_ = 0;
          tima_overflow_ = true;
          overflow_cycles_ = 0;
        } else {
          tima_++;
        }
      }
    }
    
    // Update registers
    write_registers();
    prev_div_value_ = 0;
  }
  
  // Called when TAC is written to
  void tac_updated(std::uint8_t old_tac, std::uint8_t new_tac) {
    // Save old values
    const auto old_enabled = (old_tac & (1U << TAC_ENABLE)) != 0;
    const auto old_clock_select = old_tac & 0x03U;
    
    // Get new values
    const auto new_enabled = (new_tac & (1U << TAC_ENABLE)) != 0;
    const auto new_clock_select = new_tac & 0x03U;
    
    // If the timer is disabled, do nothing
    if (!old_enabled && !new_enabled) {
      return;
    }
    
    // Read current registers
    read_registers();
    
    if (old_enabled && !new_enabled) {
      // Timer was disabled - no edge can occur
      // Set the TAC value
      tac_ = new_tac;
    } else if (old_clock_select != new_clock_select || (!old_enabled && new_enabled)) {
      // Clock select changed or timer was enabled
      
      // Set the new TAC value
      tac_ = new_tac;
      
      // Get the old and new input bits
      constexpr std::uint16_t old_bit_positions[] = {
        1U << 9, 1U << 3, 1U << 5, 1U << 7
      };
      const auto old_input_bit = old_bit_positions[old_clock_select];
      
      constexpr std::uint16_t new_bit_positions[] = {
        1U << 9, 1U << 3, 1U << 5, 1U << 7
      };
      const auto new_input_bit = new_bit_positions[new_clock_select];
      
      // Check if a falling edge is detected
      if (new_enabled && 
          ((div_internal_ & old_input_bit) != 0) &&
          ((div_internal_ & new_input_bit) == 0)) {
        
        // Increment TIMA on falling edge
        if (tima_ == 0xFF) {
          // TIMA overflow
          tima_ = 0;
          tima_overflow_ = true;
          overflow_cycles_ = 0;
        } else {
          tima_++;
        }
      }
    }
    
    // Update registers
    write_registers();
  }
};

} // namespace cycle_perfect_gameboy::system

#endif // CYCLE_PERFECT_GAMEBOY_SYSTEM_TIMER_HPP