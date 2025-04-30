#include <catch2/catch_test_macros.hpp>

#include <cycle_perfect_gameboy/system/interrupt_controller.hpp>
#include <cycle_perfect_gameboy/memory/memory_map.hpp>
#include <cycle_perfect_gameboy/memory/ram.hpp>

namespace cpgb = cycle_perfect_gameboy;

// Custom memory interface for testing (reused from timer test)
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

TEST_CASE("InterruptController can manage interrupts", "[system][interrupts]") {
  // NOLINTBEGIN(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  // Create memory and interrupt controller
  MockMemory memory;
  cpgb::system::InterruptController interrupt_controller(memory);
  
  SECTION("Initialization sets correct default values") {
    interrupt_controller.initialize();
    
    // Check initial register values
    REQUIRE(memory.read(cpgb::system::IF_ADDR) == 0xE1);
    REQUIRE(memory.read(cpgb::system::IE_ADDR) == 0x00);
  }
  
  SECTION("Requesting an interrupt sets its flag") {
    interrupt_controller.initialize();
    
    // Request VBlank interrupt
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Check IF register
    REQUIRE((memory.read(cpgb::system::IF_ADDR) & (1U << cpgb::system::INTERRUPT_VBLANK)) != 0);
  }
  
  SECTION("Enabling an interrupt sets its enable bit") {
    interrupt_controller.initialize();
    
    // Enable VBlank interrupt
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Check IE register
    REQUIRE((memory.read(cpgb::system::IE_ADDR) & (1U << cpgb::system::INTERRUPT_VBLANK)) != 0);
  }
  
  SECTION("Disabling an interrupt clears its enable bit") {
    interrupt_controller.initialize();
    
    // Enable VBlank interrupt
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Disable VBlank interrupt
    interrupt_controller.disable_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Check IE register
    REQUIRE((memory.read(cpgb::system::IE_ADDR) & (1U << cpgb::system::INTERRUPT_VBLANK)) == 0);
  }
  
  SECTION("Clearing an interrupt flag works") {
    interrupt_controller.initialize();
    
    // Request VBlank interrupt
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Clear VBlank interrupt
    interrupt_controller.clear_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Check IF register
    REQUIRE((memory.read(cpgb::system::IF_ADDR) & (1U << cpgb::system::INTERRUPT_VBLANK)) == 0);
  }
  
  SECTION("Interrupts are only pending when both requested and enabled") {
    interrupt_controller.initialize();
    
    // Request VBlank and LCD STAT interrupts
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_VBLANK);
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_LCD_STAT);
    
    // Only enable VBlank interrupt
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_VBLANK);
    
    // Check pending interrupts
    REQUIRE(interrupt_controller.interrupts_pending() == true);
    
    // Get interrupt vector (should be VBlank)
    REQUIRE(interrupt_controller.get_interrupt_vector() == cpgb::system::VBLANK_VECTOR);
    
    // VBlank flag should now be cleared
    REQUIRE((memory.read(cpgb::system::IF_ADDR) & (1U << cpgb::system::INTERRUPT_VBLANK)) == 0);
    
    // LCD STAT flag should still be set
    REQUIRE((memory.read(cpgb::system::IF_ADDR) & (1U << cpgb::system::INTERRUPT_LCD_STAT)) != 0);
  }
  
  SECTION("Interrupt priority is respected") {
    interrupt_controller.initialize();
    
    // Request all interrupts
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_VBLANK);
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_LCD_STAT);
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_TIMER);
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_SERIAL);
    interrupt_controller.request_interrupt(cpgb::system::INTERRUPT_JOYPAD);
    
    // Enable all interrupts
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_VBLANK);
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_LCD_STAT);
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_TIMER);
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_SERIAL);
    interrupt_controller.enable_interrupt(cpgb::system::INTERRUPT_JOYPAD);
    
    // Get interrupt vector (should be VBlank, highest priority)
    REQUIRE(interrupt_controller.get_interrupt_vector() == cpgb::system::VBLANK_VECTOR);
    
    // VBlank flag should now be cleared
    REQUIRE((memory.read(cpgb::system::IF_ADDR) & (1U << cpgb::system::INTERRUPT_VBLANK)) == 0);
    
    // Get next interrupt vector (should be LCD STAT, second priority)
    REQUIRE(interrupt_controller.get_interrupt_vector() == cpgb::system::LCD_STAT_VECTOR);
    
    // Get next interrupt vector (should be Timer, third priority)
    REQUIRE(interrupt_controller.get_interrupt_vector() == cpgb::system::TIMER_VECTOR);
    
    // Get next interrupt vector (should be Serial, fourth priority)
    REQUIRE(interrupt_controller.get_interrupt_vector() == cpgb::system::SERIAL_VECTOR);
    
    // Get next interrupt vector (should be Joypad, lowest priority)
    REQUIRE(interrupt_controller.get_interrupt_vector() == cpgb::system::JOYPAD_VECTOR);
    
    // No more pending interrupts
    REQUIRE(interrupt_controller.interrupts_pending() == false);
    REQUIRE(interrupt_controller.get_interrupt_vector() == 0);
  }
  // NOLINTEND(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
}