#ifndef CYCLE_PERFECT_GAMEBOY_MEMORY_RAM_HPP
#define CYCLE_PERFECT_GAMEBOY_MEMORY_RAM_HPP

#include <array>
#include <cstdint>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::memory {

template <std::size_t Size>
class RAM : public MemoryInterface {
  std::array<std::uint8_t, Size> data_{};

public:
  constexpr RAM() {
    data_.fill(0);
  }
  
  [[nodiscard]] constexpr auto read(core::Address addr) const -> std::uint8_t override {
    const auto offset = static_cast<std::size_t>(addr.value());
    if (offset >= Size) {
      return 0xFF; // Read outside memory returns 0xFF (typical open bus behavior)
    }
    return data_[offset];
  }
  
  constexpr void write(core::Address addr, std::uint8_t value) override {
    const auto offset = static_cast<std::size_t>(addr.value());
    if (offset < Size) {
      data_[offset] = value;
    }
  }
};

} // namespace cycle_perfect_gameboy::memory

#endif // CYCLE_PERFECT_GAMEBOY_MEMORY_RAM_HPP