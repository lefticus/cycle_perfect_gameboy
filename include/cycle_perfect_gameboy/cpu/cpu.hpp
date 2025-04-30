#ifndef CYCLE_PERFECT_GAMEBOY_CPU_CPU_HPP
#define CYCLE_PERFECT_GAMEBOY_CPU_CPU_HPP

#include <cycle_perfect_gameboy/cpu/registers.hpp>
#include <cycle_perfect_gameboy/cpu/instructions.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::cpu {

enum class InterruptFlag : std::uint8_t {
  VBlank    = 0, // Bit 0: VBlank interrupt
  LCDStat   = 1, // Bit 1: LCD STAT interrupt
  Timer     = 2, // Bit 2: Timer interrupt
  Serial    = 3, // Bit 3: Serial transfer completion interrupt
  Joypad    = 4  // Bit 4: Joypad input interrupt
};

class CPU {
private:
  Registers registers_{};
  bool ime_{false}; // Interrupt Master Enable flag
  bool halted_{false};
  bool stopped_{false};
  
  // Memory interface reference - CPU doesn't own this, system does
  const memory::MemoryInterface& memory_;
  memory::MemoryInterface& mutable_memory_;

public:
  constexpr CPU(const memory::MemoryInterface& memory, memory::MemoryInterface& mutable_memory)
    : memory_(memory), mutable_memory_(mutable_memory) {}
  
  // Initialize CPU state
  constexpr void initialize() {
    // Set initial register values according to GameBoy specification
    registers_.set_af(0x01B0);
    registers_.set_bc(0x0013);
    registers_.set_de(0x00D8);
    registers_.set_hl(0x014D);
    registers_.set_sp(0xFFFE);
    registers_.set_pc(0x0100); // After boot ROM, PC starts at 0x0100
    
    ime_ = false;
    halted_ = false;
    stopped_ = false;
  }
  
  // Execute a single instruction and return the number of cycles it took
  [[nodiscard]] constexpr auto execute_instruction() -> core::Cycles {
    if (halted_) {
      return core::Cycles{4}; // While halted, each "instruction" takes 4 cycles
    }
    
    if (stopped_) {
      return core::Cycles{4}; // Similarly for stopped state
    }
    
    // Check for interrupts
    if (ime_) {
      const auto interrupt_cycles = handle_interrupts();
      if (interrupt_cycles.value() > 0) {
        return interrupt_cycles;
      }
    }
    
    // Read opcode at PC
    auto pc = registers_.pc_.to_address();
    const auto opcode = memory_.read(pc);
    
    // Execute instruction
    if (opcode == 0xCB) {
      // CB-prefixed instruction - advance PC past CB prefix
      registers_.set_pc(core::RegisterPair{static_cast<std::uint16_t>(registers_.pc_.get() + 1)});
      
      // CB instruction - placeholder for now
      // Will need to read the CB opcode later: memory_.read(core::Address{static_cast<std::uint16_t>(pc.value() + 1)})
      
      // Execute CB instruction - placeholder for now
      return core::Cycles{4}; // Default to 4 cycles for now
    } else {
      // Regular instruction
      const auto& instruction = instruction_table[opcode];
      
      if (instruction.handler) {
        // Call the handler with appropriate parameters
        // Note: PC is now passed by reference and can be modified by handlers
        const auto cycles = instruction.handler(registers_, mutable_memory_, pc);
        // Update registers_.pc_ to the value updated by the handler
        registers_.set_pc(core::RegisterPair{pc.value()});
        return cycles;
      } else {
        // For unimplemented instructions, at least advance PC by the instruction length
        registers_.set_pc(core::RegisterPair{static_cast<std::uint16_t>(
            registers_.pc_.get() + instruction.length)});
        return core::Cycles{4}; // Default to 4 cycles
      }
    }
    
    // Should never reach here
    return core::Cycles{4};
  }
  
  // Run for a specified number of cycles
  constexpr void run(std::uint32_t cycles) {
    std::uint32_t cycles_executed = 0;
    while (cycles_executed < cycles) {
      const auto instruction_cycles = execute_instruction();
      cycles_executed += instruction_cycles.value();
    }
  }
  
  // Check and handle pending interrupts
  [[nodiscard]] constexpr auto handle_interrupts() -> core::Cycles {
    const auto interrupt_flag = memory_.read(core::Address{0xFF0F});
    const auto interrupt_enable = memory_.read(core::Address{0xFFFF});
    
    // No interrupts pending or enabled
    if ((interrupt_flag & interrupt_enable & 0x1F) == 0) {
      return core::Cycles{0};
    }
    
    // Interrupts are pending and enabled
    halted_ = false; // Exit HALT state
    
    // If IME is set, handle the highest priority interrupt
    if (ime_) {
      ime_ = false; // Disable interrupts while handling one
      
      const auto pending_interrupts = interrupt_flag & interrupt_enable & 0x1F;
      
      for (std::uint8_t bit = 0; bit < 5; ++bit) {
        if ((pending_interrupts & static_cast<std::uint8_t>(1U << bit)) != 0) {
          // Clear this interrupt flag
          mutable_memory_.write(core::Address{0xFF0F}, static_cast<std::uint8_t>(interrupt_flag & ~static_cast<std::uint8_t>(1U << bit)));
          
          // Push PC onto stack
          registers_.set_sp(registers_.sp_.get() - 2);
          const auto sp = registers_.sp_.to_address();
          mutable_memory_.write(sp, static_cast<std::uint8_t>(registers_.pc_.get() & 0xFF));
          mutable_memory_.write(sp + core::Address{1}, static_cast<std::uint8_t>(registers_.pc_.get() >> 8));
          
          // Jump to interrupt handler
          const std::uint16_t interrupt_vectors[] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};
          registers_.set_pc(interrupt_vectors[bit]);
          
          return core::Cycles{20}; // Interrupt handling takes 20 cycles
        }
      }
    }
    
    return core::Cycles{0};
  }
  
  // Getters for CPU state
  [[nodiscard]] constexpr auto get_registers() const noexcept -> const Registers& { return registers_; }
  [[nodiscard]] constexpr auto is_ime_enabled() const noexcept -> bool { return ime_; }
  [[nodiscard]] constexpr auto is_halted() const noexcept -> bool { return halted_; }
  [[nodiscard]] constexpr auto is_stopped() const noexcept -> bool { return stopped_; }
  
  // Control methods
  constexpr void enable_interrupts() noexcept { ime_ = true; }
  constexpr void disable_interrupts() noexcept { ime_ = false; }
  constexpr void halt() noexcept { halted_ = true; }
  constexpr void stop() noexcept { stopped_ = true; }
};

} // namespace cycle_perfect_gameboy::cpu

#endif // CYCLE_PERFECT_GAMEBOY_CPU_CPU_HPP