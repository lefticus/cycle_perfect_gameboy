#ifndef CYCLE_PERFECT_GAMEBOY_CPU_REGISTERS_HPP
#define CYCLE_PERFECT_GAMEBOY_CPU_REGISTERS_HPP

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/cpu/flags.hpp>

namespace cycle_perfect_gameboy::cpu {

class Registers {
public:
  // Main register pairs
  core::RegisterPair af_{}; // A (accumulator) and F (flags)
  core::RegisterPair bc_{}; // B and C general purpose
  core::RegisterPair de_{}; // D and E general purpose
  core::RegisterPair hl_{}; // H and L, often used as a pointer
  
  // Special registers
  core::RegisterPair sp_{}; // Stack Pointer
  core::RegisterPair pc_{}; // Program Counter
  
  // Easy access to individual registers
  [[nodiscard]] constexpr auto a() const noexcept -> core::Register { return af_.high(); }
  [[nodiscard]] constexpr auto f() const noexcept -> FlagRegister { return FlagRegister{af_.low().get()}; }
  [[nodiscard]] constexpr auto b() const noexcept -> core::Register { return bc_.high(); }
  [[nodiscard]] constexpr auto c() const noexcept -> core::Register { return bc_.low(); }
  [[nodiscard]] constexpr auto d() const noexcept -> core::Register { return de_.high(); }
  [[nodiscard]] constexpr auto e() const noexcept -> core::Register { return de_.low(); }
  [[nodiscard]] constexpr auto h() const noexcept -> core::Register { return hl_.high(); }
  [[nodiscard]] constexpr auto l() const noexcept -> core::Register { return hl_.low(); }
  
  // Register pair getters
  [[nodiscard]] constexpr auto get_af() const noexcept -> const core::RegisterPair& { return af_; }
  [[nodiscard]] constexpr auto get_bc() const noexcept -> const core::RegisterPair& { return bc_; }
  [[nodiscard]] constexpr auto get_de() const noexcept -> const core::RegisterPair& { return de_; }
  [[nodiscard]] constexpr auto get_hl() const noexcept -> const core::RegisterPair& { return hl_; }
  [[nodiscard]] constexpr auto get_sp() const noexcept -> const core::RegisterPair& { return sp_; }
  [[nodiscard]] constexpr auto get_pc() const noexcept -> const core::RegisterPair& { return pc_; }
  
  // Register pair setters
  constexpr void set_af(std::uint16_t value) noexcept { af_.set(value & 0xFFF0); } // Lower 4 bits of F are always 0
  constexpr void set_bc(std::uint16_t value) noexcept { bc_.set(value); }
  constexpr void set_de(std::uint16_t value) noexcept { de_.set(value); }
  constexpr void set_hl(std::uint16_t value) noexcept { hl_.set(value); }
  constexpr void set_sp(std::uint16_t value) noexcept { sp_.set(value); }
  constexpr void set_pc(std::uint16_t value) noexcept { pc_.set(value); }
  
  // Register pair setters that take RegisterPair
  constexpr void set_af(const core::RegisterPair& value) noexcept { af_.set(value.value() & 0xFFF0); }
  constexpr void set_bc(const core::RegisterPair& value) noexcept { bc_.set(value.value()); }
  constexpr void set_de(const core::RegisterPair& value) noexcept { de_.set(value.value()); }
  constexpr void set_hl(const core::RegisterPair& value) noexcept { hl_.set(value.value()); }
  constexpr void set_sp(const core::RegisterPair& value) noexcept { sp_.set(value.value()); }
  constexpr void set_pc(const core::RegisterPair& value) noexcept { pc_.set(value.value()); }
  
  // Individual register getters
  [[nodiscard]] constexpr auto get_a() const noexcept -> std::uint8_t { return af_.high().get(); }
  [[nodiscard]] constexpr auto get_f() const noexcept -> std::uint8_t { return af_.low().get(); }
  [[nodiscard]] constexpr auto get_b() const noexcept -> std::uint8_t { return bc_.high().get(); }
  [[nodiscard]] constexpr auto get_c() const noexcept -> std::uint8_t { return bc_.low().get(); }
  [[nodiscard]] constexpr auto get_d() const noexcept -> std::uint8_t { return de_.high().get(); }
  [[nodiscard]] constexpr auto get_e() const noexcept -> std::uint8_t { return de_.low().get(); }
  [[nodiscard]] constexpr auto get_h() const noexcept -> std::uint8_t { return hl_.high().get(); }
  [[nodiscard]] constexpr auto get_l() const noexcept -> std::uint8_t { return hl_.low().get(); }
  
  // Individual register setters
  constexpr void set_a(std::uint8_t value) noexcept { af_.set_high(value); }
  constexpr void set_f(std::uint8_t value) noexcept { af_.set_low(value & 0xF0); } // Lower 4 bits always 0
  constexpr void set_b(std::uint8_t value) noexcept { bc_.set_high(value); }
  constexpr void set_c(std::uint8_t value) noexcept { bc_.set_low(value); }
  constexpr void set_d(std::uint8_t value) noexcept { de_.set_high(value); }
  constexpr void set_e(std::uint8_t value) noexcept { de_.set_low(value); }
  constexpr void set_h(std::uint8_t value) noexcept { hl_.set_high(value); }
  constexpr void set_l(std::uint8_t value) noexcept { hl_.set_low(value); }
  
  // Flag register convenience methods
  [[nodiscard]] constexpr auto flag_zero() const noexcept -> bool { return f().zero(); }
  [[nodiscard]] constexpr auto flag_subtract() const noexcept -> bool { return f().subtract(); }
  [[nodiscard]] constexpr auto flag_half_carry() const noexcept -> bool { return f().half_carry(); }
  [[nodiscard]] constexpr auto flag_carry() const noexcept -> bool { return f().carry(); }
  
  // Alternative names for flag getters
  [[nodiscard]] constexpr auto get_zero_flag() const noexcept -> bool { return flag_zero(); }
  [[nodiscard]] constexpr auto get_subtract_flag() const noexcept -> bool { return flag_subtract(); }
  [[nodiscard]] constexpr auto get_half_carry_flag() const noexcept -> bool { return flag_half_carry(); }
  [[nodiscard]] constexpr auto get_carry_flag() const noexcept -> bool { return flag_carry(); }
  
  constexpr void set_flag_zero(bool value) noexcept {
    auto flag_reg = f();
    flag_reg.set_zero(value);
    set_f(flag_reg.get());
  }
  
  constexpr void set_flag_subtract(bool value) noexcept {
    auto flag_reg = f();
    flag_reg.set_subtract(value);
    set_f(flag_reg.get());
  }
  
  constexpr void set_flag_half_carry(bool value) noexcept {
    auto flag_reg = f();
    flag_reg.set_half_carry(value);
    set_f(flag_reg.get());
  }
  
  constexpr void set_flag_carry(bool value) noexcept {
    auto flag_reg = f();
    flag_reg.set_carry(value);
    set_f(flag_reg.get());
  }
  
  // Alternative names for flag setters
  constexpr void set_zero_flag(bool value) noexcept { set_flag_zero(value); }
  constexpr void set_subtract_flag(bool value) noexcept { set_flag_subtract(value); }
  constexpr void set_half_carry_flag(bool value) noexcept { set_flag_half_carry(value); }
  constexpr void set_carry_flag(bool value) noexcept { set_flag_carry(value); }
};

} // namespace cycle_perfect_gameboy::cpu

#endif // CYCLE_PERFECT_GAMEBOY_CPU_REGISTERS_HPP