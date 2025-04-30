#ifndef CYCLE_PERFECT_GAMEBOY_CPU_FLAGS_HPP
#define CYCLE_PERFECT_GAMEBOY_CPU_FLAGS_HPP

#include <cstdint>

namespace cycle_perfect_gameboy::cpu {

enum class Flag : std::uint8_t {
  Zero = 7,       // Set when the result of an operation is zero
  Subtract = 6,   // Set when the last operation was a subtraction
  HalfCarry = 5,  // Set when the lower half of the byte overflowed past 15
  Carry = 4       // Set when the result overflowed past 255 (for additions) or was below 0 (for subtractions)
};

class FlagRegister {
  std::uint8_t value_{0};

public:
  constexpr FlagRegister() noexcept = default;
  constexpr explicit FlagRegister(std::uint8_t value) noexcept : value_{value} {}
  
  [[nodiscard]] constexpr auto get() const noexcept -> std::uint8_t { return value_; }
  constexpr void set(std::uint8_t value) noexcept { value_ = value & 0xF0; } // Lower 4 bits always read as 0
  
  [[nodiscard]] constexpr auto zero() const noexcept -> bool { return (value_ & static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Zero))) != 0; }
  [[nodiscard]] constexpr auto subtract() const noexcept -> bool { return (value_ & static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Subtract))) != 0; }
  [[nodiscard]] constexpr auto half_carry() const noexcept -> bool { return (value_ & static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::HalfCarry))) != 0; }
  [[nodiscard]] constexpr auto carry() const noexcept -> bool { return (value_ & static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Carry))) != 0; }
  
  constexpr void set_zero(bool value) noexcept {
    if (value) {
      value_ |= static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Zero));
    } else {
      value_ &= static_cast<std::uint8_t>(~static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Zero)));
    }
  }
  
  constexpr void set_subtract(bool value) noexcept {
    if (value) {
      value_ |= static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Subtract));
    } else {
      value_ &= static_cast<std::uint8_t>(~static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Subtract)));
    }
  }
  
  constexpr void set_half_carry(bool value) noexcept {
    if (value) {
      value_ |= static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::HalfCarry));
    } else {
      value_ &= static_cast<std::uint8_t>(~static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::HalfCarry)));
    }
  }
  
  constexpr void set_carry(bool value) noexcept {
    if (value) {
      value_ |= static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Carry));
    } else {
      value_ &= static_cast<std::uint8_t>(~static_cast<std::uint8_t>(1U << static_cast<std::uint8_t>(Flag::Carry)));
    }
  }
};

} // namespace cycle_perfect_gameboy::cpu

#endif // CYCLE_PERFECT_GAMEBOY_CPU_FLAGS_HPP