#include <catch2/catch_test_macros.hpp>

#include <cycle_perfect_gameboy/core/types.hpp>

namespace cpgb = cycle_perfect_gameboy::core;

TEST_CASE("Address can be created and manipulated", "[core][types]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  constexpr cpgb::Address addr{0x1234};
  
  SECTION("Value is stored correctly") {
    REQUIRE(addr.value() == 0x1234);
  }
  
  SECTION("Addition works") {
    constexpr auto result = addr + 0x10;
    REQUIRE(result.value() == 0x1244);
  }
  
  SECTION("Subtraction works") {
    constexpr auto result = addr - 0x10;
    REQUIRE(result.value() == 0x1224);
  }
  
  SECTION("Comparison operators work") {
    constexpr cpgb::Address lower{0x1000};
    constexpr cpgb::Address same{0x1234};
    constexpr cpgb::Address higher{0x2000};
    
    REQUIRE(addr == same);
    REQUIRE(addr != lower);
    REQUIRE(addr != higher);
    REQUIRE(addr > lower);
    REQUIRE(addr < higher);
    REQUIRE(addr >= lower);
    REQUIRE(addr >= same);
    REQUIRE(addr <= higher);
    REQUIRE(addr <= same);
  }
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}

TEST_CASE("Register can store and retrieve values", "[core][types]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  cpgb::Register reg{0x42};
  
  SECTION("Initial value is stored correctly") {
    REQUIRE(reg.get() == 0x42);
  }
  
  SECTION("Value can be changed") {
    reg.set(0x24);
    REQUIRE(reg.get() == 0x24);
  }
  
  SECTION("Equality comparison works") {
    const cpgb::Register same{0x42};
    const cpgb::Register different{0x24};
    
    REQUIRE(reg == same);
    REQUIRE(reg != different);
  }
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}

TEST_CASE("RegisterPair can store and access 16-bit values", "[core][types]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  cpgb::RegisterPair pair{0x1234};
  
  SECTION("Value is stored correctly") {
    REQUIRE(pair.get() == 0x1234);
  }
  
  SECTION("High byte is correct") {
    REQUIRE(pair.high().get() == 0x12);
  }
  
  SECTION("Low byte is correct") {
    REQUIRE(pair.low().get() == 0x34);
  }
  
  SECTION("Can be constructed from high/low registers") {
    const cpgb::Register high{0x12};
    const cpgb::Register low{0x34};
    const cpgb::RegisterPair from_registers{high, low};
    
    REQUIRE(from_registers.get() == 0x1234);
  }
  
  SECTION("Can modify high byte") {
    pair.set_high(0x56);
    REQUIRE(pair.get() == 0x5634);
  }
  
  SECTION("Can modify low byte") {
    pair.set_low(0x78);
    REQUIRE(pair.get() == 0x1278);
  }
  
  SECTION("Can convert to address") {
    const auto addr = pair.to_address();
    REQUIRE(addr.value() == 0x1234);
  }
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}

TEST_CASE("Cycles can be created and combined", "[core][types]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  constexpr cpgb::Cycles four{4};
  constexpr cpgb::Cycles eight{8};
  
  SECTION("Value is stored correctly") {
    REQUIRE(four.value() == 4);
    REQUIRE(eight.value() == 8);
  }
  
  SECTION("Addition works") {
    constexpr auto combined = four + eight;
    REQUIRE(combined.value() == 12);
  }
  
  SECTION("Equality comparison works") {
    constexpr cpgb::Cycles also_four{4};
    
    REQUIRE(four == also_four);
    REQUIRE(four != eight);
  }
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}