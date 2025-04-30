#ifndef CYCLE_PERFECT_GAMEBOY_CORE_TYPES_HPP
#define CYCLE_PERFECT_GAMEBOY_CORE_TYPES_HPP

#include <cstdint>

namespace cycle_perfect_gameboy::core {

class Address {
  std::uint16_t value_{0};

public:
  constexpr explicit Address(std::uint16_t value) noexcept : value_{value} {}
  
  [[nodiscard]] constexpr auto value() const noexcept -> std::uint16_t { return value_; }
  [[nodiscard]] constexpr auto operator==(const Address& other) const noexcept -> bool { return value_ == other.value_; }
  [[nodiscard]] constexpr auto operator!=(const Address& other) const noexcept -> bool { return value_ != other.value_; }
  [[nodiscard]] constexpr auto operator<(const Address& other) const noexcept -> bool { return value_ < other.value_; }
  [[nodiscard]] constexpr auto operator<=(const Address& other) const noexcept -> bool { return value_ <= other.value_; }
  [[nodiscard]] constexpr auto operator>(const Address& other) const noexcept -> bool { return value_ > other.value_; }
  [[nodiscard]] constexpr auto operator>=(const Address& other) const noexcept -> bool { return value_ >= other.value_; }
  [[nodiscard]] constexpr auto operator+(std::uint16_t offset) const noexcept -> Address { return Address{static_cast<std::uint16_t>(value_ + offset)}; }
  [[nodiscard]] constexpr auto operator+(const Address& other) const noexcept -> Address { return Address{static_cast<std::uint16_t>(value_ + other.value_)}; }
  [[nodiscard]] constexpr auto operator-(std::uint16_t offset) const noexcept -> Address { return Address{static_cast<std::uint16_t>(value_ - offset)}; }
};

class Cycles {
  std::uint8_t value_{0};

public:
  constexpr explicit Cycles(std::uint8_t value) noexcept : value_{value} {}
  
  [[nodiscard]] constexpr auto value() const noexcept -> std::uint8_t { return value_; }
  [[nodiscard]] constexpr auto operator+(const Cycles& other) const noexcept -> Cycles { return Cycles{static_cast<std::uint8_t>(value_ + other.value_)}; }
  [[nodiscard]] constexpr auto operator==(const Cycles& other) const noexcept -> bool { return value_ == other.value_; }
};

class Register {
  std::uint8_t value_{0};

public:
  constexpr Register() noexcept = default;
  constexpr explicit Register(std::uint8_t value) noexcept : value_{value} {}
  
  [[nodiscard]] constexpr auto get() const noexcept -> std::uint8_t { return value_; }
  constexpr void set(std::uint8_t value) noexcept { value_ = value; }
  
  [[nodiscard]] constexpr auto operator==(const Register& other) const noexcept -> bool { return value_ == other.get(); }
};

class RegisterPair {
  std::uint16_t value_{0};

public:
  constexpr RegisterPair() noexcept = default;
  constexpr explicit RegisterPair(std::uint16_t value) noexcept : value_{value} {}
  constexpr RegisterPair(Register high, Register low) noexcept
    : value_{static_cast<std::uint16_t>((static_cast<std::uint16_t>(high.get()) << 8) | low.get())} {}
  
  [[nodiscard]] constexpr auto get() const noexcept -> std::uint16_t { return value_; }
  constexpr void set(std::uint16_t value) noexcept { value_ = value; }
  
  [[nodiscard]] constexpr auto high() const noexcept -> Register { return Register{static_cast<std::uint8_t>(value_ >> 8)}; }
  [[nodiscard]] constexpr auto low() const noexcept -> Register { return Register{static_cast<std::uint8_t>(value_ & 0xFF)}; }
  
  constexpr void set_high(std::uint8_t value) noexcept { value_ = (value_ & 0x00FFU) | (static_cast<std::uint16_t>(static_cast<std::uint16_t>(value) << 8U)); }
  constexpr void set_low(std::uint8_t value) noexcept { value_ = (value_ & 0xFF00U) | value; }
  
  [[nodiscard]] constexpr auto to_address() const noexcept -> Address { return Address{value_}; }
  [[nodiscard]] constexpr auto operator==(const RegisterPair& other) const noexcept -> bool { return value_ == other.get(); }
};

} // namespace cycle_perfect_gameboy::core

#endif // CYCLE_PERFECT_GAMEBOY_CORE_TYPES_HPP