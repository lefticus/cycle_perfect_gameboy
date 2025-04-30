#ifndef CYCLE_PERFECT_GAMEBOY_MEMORY_INTERFACE_HPP
#define CYCLE_PERFECT_GAMEBOY_MEMORY_INTERFACE_HPP

#include <cycle_perfect_gameboy/core/types.hpp>

namespace cycle_perfect_gameboy::memory {

class MemoryInterface {
public:
  [[nodiscard]] virtual constexpr auto read(core::Address addr) const -> std::uint8_t = 0;
  virtual constexpr void write(core::Address addr, std::uint8_t value) = 0;
  virtual constexpr ~MemoryInterface() = default;
};

} // namespace cycle_perfect_gameboy::memory

#endif // CYCLE_PERFECT_GAMEBOY_MEMORY_INTERFACE_HPP