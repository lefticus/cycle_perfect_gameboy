#ifndef CYCLE_PERFECT_GAMEBOY_CARTRIDGE_ROM_HPP
#define CYCLE_PERFECT_GAMEBOY_CARTRIDGE_ROM_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <cycle_perfect_gameboy/core/types.hpp>
#include <cycle_perfect_gameboy/memory/memory_interface.hpp>
#include <cycle_perfect_gameboy/cartridge/cartridge_header.hpp>

namespace cycle_perfect_gameboy::cartridge {

class ROM : public memory::MemoryInterface {
private:
  std::vector<std::uint8_t> data_{};
  CartridgeHeader header_{};

public:
  ROM() = default;
  
  [[nodiscard]] bool load_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
      return false;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read file contents
    data_.resize(static_cast<std::size_t>(size));
    file.read(reinterpret_cast<char*>(data_.data()), size);
    
    if (!file) {
      data_.clear();
      return false;
    }
    
    // Parse header
    if (data_.size() >= 0x150) {
      header_.parse_data(data_.data());
    }
    
    return true;
  }
  
  [[nodiscard]] constexpr auto read(core::Address addr) const -> std::uint8_t override {
    const auto offset = static_cast<std::size_t>(addr.value());
    if (offset < data_.size()) {
      return data_[offset];
    }
    return 0xFF; // Return 0xFF for out-of-bounds reads
  }
  
  constexpr void write([[maybe_unused]] core::Address addr, [[maybe_unused]] std::uint8_t value) override {
    // ROM is generally not writable, but some MBC operations are triggered by writes
    // For basic ROM, we ignore writes
  }
  
  [[nodiscard]] constexpr auto get_header() const -> const CartridgeHeader& {
    return header_;
  }
  
  [[nodiscard]] constexpr auto get_size() const -> std::size_t {
    return data_.size();
  }
  
  // Static factory method to create ROM from file
  [[nodiscard]] static auto from_file(const std::filesystem::path& path) -> std::optional<ROM> {
    ROM rom;
    if (rom.load_from_file(path.string())) {
      return std::make_optional(std::move(rom));
    }
    return std::nullopt;
  }
};

} // namespace cycle_perfect_gameboy::cartridge

#endif // CYCLE_PERFECT_GAMEBOY_CARTRIDGE_ROM_HPP