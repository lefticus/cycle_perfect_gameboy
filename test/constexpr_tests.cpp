#include <catch2/catch_test_macros.hpp>

#include <cycle_perfect_gameboy/sample_library.hpp>
#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/cpu/flags.hpp>
#include <cycle_perfect_gameboy/cpu/registers.hpp>

namespace cpgb = cycle_perfect_gameboy::core;
namespace cpgb_cpu = cycle_perfect_gameboy::cpu;

TEST_CASE("Factorials are computed with constexpr", "[factorial]")
{
  STATIC_REQUIRE(factorial_constexpr(0) == 1);
  STATIC_REQUIRE(factorial_constexpr(1) == 1);
  STATIC_REQUIRE(factorial_constexpr(2) == 2);
  STATIC_REQUIRE(factorial_constexpr(3) == 6);
  STATIC_REQUIRE(factorial_constexpr(10) == 3628800);
}

TEST_CASE("Constexpr Address operations work", "[constexpr][core][types]") {
  constexpr cpgb::Address addr{0x1234};
  
  STATIC_REQUIRE(addr.value() == 0x1234);
  
  constexpr auto addr_plus = addr + 0x10;
  STATIC_REQUIRE(addr_plus.value() == 0x1244);
  
  constexpr auto addr_minus = addr - 0x10;
  STATIC_REQUIRE(addr_minus.value() == 0x1224);
  
  constexpr cpgb::Address same{0x1234};
  constexpr cpgb::Address different{0x4321};
  
  STATIC_REQUIRE(addr == same);
  STATIC_REQUIRE(addr != different);
}

TEST_CASE("Constexpr Register operations work", "[constexpr][core][types]") {
  constexpr cpgb::Register reg{0x42};
  
  STATIC_REQUIRE(reg.get() == 0x42);
  
  constexpr cpgb::Register same{0x42};
  constexpr cpgb::Register different{0x24};
  
  STATIC_REQUIRE(reg == same);
  STATIC_REQUIRE(reg != different);
}

TEST_CASE("Constexpr RegisterPair operations work", "[constexpr][core][types]") {
  constexpr cpgb::RegisterPair pair{0x1234};
  
  STATIC_REQUIRE(pair.get() == 0x1234);
  STATIC_REQUIRE(pair.high().get() == 0x12);
  STATIC_REQUIRE(pair.low().get() == 0x34);
  
  constexpr cpgb::Register high{0x12};
  constexpr cpgb::Register low{0x34};
  constexpr cpgb::RegisterPair from_registers{high, low};
  
  STATIC_REQUIRE(from_registers.get() == 0x1234);
  
  constexpr auto address = pair.to_address();
  STATIC_REQUIRE(address.value() == 0x1234);
}

TEST_CASE("Constexpr Cycles operations work", "[constexpr][core][types]") {
  constexpr cpgb::Cycles four{4};
  constexpr cpgb::Cycles eight{8};
  
  STATIC_REQUIRE(four.value() == 4);
  STATIC_REQUIRE(eight.value() == 8);
  
  constexpr auto combined = four + eight;
  STATIC_REQUIRE(combined.value() == 12);
  
  constexpr cpgb::Cycles also_four{4};
  STATIC_REQUIRE(four == also_four);
  STATIC_REQUIRE(four != eight);
}

TEST_CASE("Constexpr FlagRegister operations work", "[constexpr][cpu][flags]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers, readability-identifier-length)
  constexpr cpgb_cpu::FlagRegister flags{0b10100000}; // Zero and HalfCarry set
  
  STATIC_REQUIRE(flags.zero() == true);
  STATIC_REQUIRE(flags.subtract() == false);
  STATIC_REQUIRE(flags.half_carry() == true);
  STATIC_REQUIRE(flags.carry() == false);
  
  // Test setting flags
  constexpr auto set_flags = []() {
    cpgb_cpu::FlagRegister flag_reg{0};
    flag_reg.set_zero(true);
    flag_reg.set_subtract(true);
    flag_reg.set_half_carry(false);
    flag_reg.set_carry(true);
    return flag_reg;
  };
  
  constexpr auto result = set_flags();
  STATIC_REQUIRE(result.get() == 0b11010000);
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers, readability-identifier-length)
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Constexpr Registers operations work", "[constexpr][cpu][registers]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  constexpr auto setup_registers = []() {
    cpgb_cpu::Registers regs;
    regs.set_af(0x1234);
    regs.set_bc(0x5678);
    regs.set_de(0x9ABC);
    regs.set_hl(0xDEF0);
    regs.set_sp(0x1122);
    regs.set_pc(0x3344);
    return regs;
  };
  
  constexpr auto regs = setup_registers();
  
  // Test register pairs
  STATIC_REQUIRE(regs.af_.get() == 0x1230); // Lower 4 bits of F are always 0
  STATIC_REQUIRE(regs.bc_.get() == 0x5678);
  STATIC_REQUIRE(regs.de_.get() == 0x9ABC);
  STATIC_REQUIRE(regs.hl_.get() == 0xDEF0);
  STATIC_REQUIRE(regs.sp_.get() == 0x1122);
  STATIC_REQUIRE(regs.pc_.get() == 0x3344);
  
  // Test individual registers
  STATIC_REQUIRE(regs.a().get() == 0x12);
  STATIC_REQUIRE(regs.f().get() == 0x30);
  STATIC_REQUIRE(regs.b().get() == 0x56);
  STATIC_REQUIRE(regs.c().get() == 0x78);
  STATIC_REQUIRE(regs.d().get() == 0x9A);
  STATIC_REQUIRE(regs.e().get() == 0xBC);
  STATIC_REQUIRE(regs.h().get() == 0xDE);
  STATIC_REQUIRE(regs.l().get() == 0xF0);
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}
// NOLINTEND(readability-function-cognitive-complexity)