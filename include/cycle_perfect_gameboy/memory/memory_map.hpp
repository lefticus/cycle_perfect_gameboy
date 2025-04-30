#ifndef CYCLE_PERFECT_GAMEBOY_MEMORY_MAP_HPP
#define CYCLE_PERFECT_GAMEBOY_MEMORY_MAP_HPP

#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <ranges>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>

namespace cycle_perfect_gameboy::memory {

class MemoryMap : public MemoryInterface {
public:
  struct MemoryRegion {
    core::Address start{0};
    core::Address end{0};
    std::shared_ptr<MemoryInterface> handler{nullptr};
    
    [[nodiscard]] constexpr auto contains(core::Address addr) const noexcept -> bool {
      return addr >= start && addr <= end;
    }
  };

private:
  std::vector<MemoryRegion> regions_{};

public:
  MemoryMap() = default;
  
  constexpr void add_region(core::Address start, core::Address end, std::shared_ptr<MemoryInterface> handler) {
    regions_.push_back(MemoryRegion{start, end, std::move(handler)});
  }
  
  [[nodiscard]] constexpr auto read(core::Address addr) const -> std::uint8_t override {
    const auto it = std::ranges::find_if(regions_, [addr](const auto& region) { return region.contains(addr); });
    if (it != regions_.end()) {
      return it->handler->read(addr);
    }
    return 0xFF; // Default when no handler found (open bus behavior)
  }
  
  constexpr void write(core::Address addr, std::uint8_t value) override {
    const auto it = std::ranges::find_if(regions_, [addr](const auto& region) { return region.contains(addr); });
    if (it != regions_.end()) {
      it->handler->write(addr, value);
      return;
    }
    // Silently ignore writes to unmapped memory
  }
};

} // namespace cycle_perfect_gameboy::memory

#endif // CYCLE_PERFECT_GAMEBOY_MEMORY_MAP_HPP