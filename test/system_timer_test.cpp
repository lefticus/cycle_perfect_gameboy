#include <catch2/catch_test_macros.hpp>

#include <cycle_perfect_gameboy/system/timer.hpp>
#include <cycle_perfect_gameboy/memory/memory_map.hpp>
#include <cycle_perfect_gameboy/memory/ram.hpp>

namespace cpgb = cycle_perfect_gameboy;

// Custom memory interface for testing
class MockMemory : public cpgb::memory::MemoryInterface {
private:
  std::array<std::uint8_t, 0x10000> memory_{};
  
public:
  MockMemory() {
    memory_.fill(0);
  }
  
  [[nodiscard]] constexpr auto read(cpgb::core::Address addr) const -> std::uint8_t override {
    return memory_[addr.value()];
  }
  
  constexpr void write(cpgb::core::Address addr, std::uint8_t value) override {
    memory_[addr.value()] = value;
  }
  
  void reset() {
    memory_.fill(0);
  }
};

TEST_CASE("Timer can be initialized and updated", "[system][timer]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  // Create memory and timer
  MockMemory memory;
  cpgb::system::Timer timer(memory);
  
  SECTION("Timer initialization sets correct default values") {
    timer.initialize();
    
    // Check initial register values
    REQUIRE(memory.read(cpgb::system::DIV_ADDR) == 0x00);
    REQUIRE(memory.read(cpgb::system::TIMA_ADDR) == 0x00);
    REQUIRE(memory.read(cpgb::system::TMA_ADDR) == 0x00);
    REQUIRE(memory.read(cpgb::system::TAC_ADDR) == 0x00);
  }
  
  SECTION("DIV register increments at fixed rate") {
    timer.initialize();
    
    // DIV increments at 16384Hz = CPU freq (4MHz) / 256
    // One machine cycle is 4 clock cycles, so 256/4 = 64 machine cycles should
    // increment DIV by 1
    
    // Run for 64 machine cycles (should increment DIV from 0 to 1)
    timer.update(cpgb::core::Cycles{64});
    REQUIRE(memory.read(cpgb::system::DIV_ADDR) == 0x01);
    
    // Run for 64 more machine cycles (should increment DIV from 1 to 2)
    timer.update(cpgb::core::Cycles{64});
    REQUIRE(memory.read(cpgb::system::DIV_ADDR) == 0x02);
    
    // Run for 256 machine cycles (should increment DIV by 4)
    timer.update(cpgb::core::Cycles{256});
    REQUIRE(memory.read(cpgb::system::DIV_ADDR) == 0x06);
  }
  
  SECTION("Writing to DIV resets it to 0") {
    timer.initialize();
    
    // Run for 256 machine cycles to get DIV to a non-zero value
    timer.update(cpgb::core::Cycles{256});
    REQUIRE(memory.read(cpgb::system::DIV_ADDR) > 0);
    
    // Reset DIV by writing to it
    timer.reset_div();
    
    // DIV should now be 0
    REQUIRE(memory.read(cpgb::system::DIV_ADDR) == 0x00);
  }
  
  SECTION("TIMA increments when timer is enabled with 4096Hz (00)") {
    timer.initialize();
    
    // Enable timer with 4096Hz input (TAC = 0x04)
    memory.write(cpgb::system::TAC_ADDR, 0x04);
    
    // TIMA should increment at 4096Hz = CPU freq (4MHz) / 1024
    // One machine cycle is 4 clock cycles, so 1024/4 = 256 machine cycles
    // should increment TIMA by 1
    
    // Run for 255 machine cycles (not enough to increment TIMA)
    timer.update(cpgb::core::Cycles{255});
    REQUIRE(memory.read(cpgb::system::TIMA_ADDR) == 0x00);
    
    // Run for 2 more machine cycles (enough to trigger increment)
    timer.update(cpgb::core::Cycles{2});
    REQUIRE(memory.read(cpgb::system::TIMA_ADDR) == 0x01);
    
    // Run for 256 more machine cycles (another increment)
    timer.update(cpgb::core::Cycles{256});
    REQUIRE(memory.read(cpgb::system::TIMA_ADDR) == 0x02);
  }
  
  SECTION("TIMA overflow generates interrupt and reloads from TMA") {
    timer.initialize();
    
    // Set TMA to a non-zero value
    memory.write(cpgb::system::TMA_ADDR, 0x42);
    
    // Enable timer
    memory.write(cpgb::system::TAC_ADDR, 0x04);
    
    // Set TIMA to just before overflow
    memory.write(cpgb::system::TIMA_ADDR, 0xFF);
    
    // Run enough cycles to cause overflow
    timer.update(cpgb::core::Cycles{512});
    
    // TIMA should now be equal to TMA
    REQUIRE(memory.read(cpgb::system::TIMA_ADDR) == 0x42);
    
    // Timer interrupt should be requested (bit 2 set in IF)
    REQUIRE((memory.read(cpgb::core::Address{0xFF0F}) & (1U << 2)) != 0);
  }
  
  SECTION("Changing TAC can cause TIMA to increment") {
    timer.initialize();
    
    // Set DIV to a value that will trigger an edge when switching frequencies
    memory.write(cpgb::system::DIV_ADDR, 0x08); // Sets bit 3
    
    // Switch from disabled to 262144Hz (bit 3 = 1 will cause a falling edge)
    memory.write(cpgb::system::TAC_ADDR, 0x05); // Enable (bit 2) + frequency 01
    
    // TIMA should increment due to the falling edge
    REQUIRE(memory.read(cpgb::system::TIMA_ADDR) == 0x01);
  }
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}