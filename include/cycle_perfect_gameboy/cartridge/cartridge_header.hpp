#ifndef CYCLE_PERFECT_GAMEBOY_CARTRIDGE_HEADER_HPP
#define CYCLE_PERFECT_GAMEBOY_CARTRIDGE_HEADER_HPP

#include <cstdint>
#include <string>
#include <array>

namespace cycle_perfect_gameboy::cartridge {

enum class CartridgeType : std::uint8_t {
  ROM_ONLY = 0x00,
  MBC1 = 0x01,
  MBC1_RAM = 0x02,
  MBC1_RAM_BATTERY = 0x03,
  MBC2 = 0x05,
  MBC2_BATTERY = 0x06,
  ROM_RAM = 0x08,
  ROM_RAM_BATTERY = 0x09,
  MMM01 = 0x0B,
  MMM01_RAM = 0x0C,
  MMM01_RAM_BATTERY = 0x0D,
  MBC3_TIMER_BATTERY = 0x0F,
  MBC3_TIMER_RAM_BATTERY = 0x10,
  MBC3 = 0x11,
  MBC3_RAM = 0x12,
  MBC3_RAM_BATTERY = 0x13,
  MBC5 = 0x19,
  MBC5_RAM = 0x1A,
  MBC5_RAM_BATTERY = 0x1B,
  MBC5_RUMBLE = 0x1C,
  MBC5_RUMBLE_RAM = 0x1D,
  MBC5_RUMBLE_RAM_BATTERY = 0x1E,
  MBC6 = 0x20,
  MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
  POCKET_CAMERA = 0xFC,
  BANDAI_TAMA5 = 0xFD,
  HUC3 = 0xFE,
  HUC1_RAM_BATTERY = 0xFF
};

enum class ROMSize : std::uint8_t {
  KB_32 = 0x00,   // 32KB (no banking)
  KB_64 = 0x01,   // 64KB (4 banks)
  KB_128 = 0x02,  // 128KB (8 banks)
  KB_256 = 0x03,  // 256KB (16 banks)
  KB_512 = 0x04,  // 512KB (32 banks)
  MB_1 = 0x05,    // 1MB (64 banks)
  MB_2 = 0x06,    // 2MB (128 banks)
  MB_4 = 0x07,    // 4MB (256 banks)
  MB_8 = 0x08     // 8MB (512 banks)
};

enum class RAMSize : std::uint8_t {
  NONE = 0x00,     // No RAM
  KB_2 = 0x01,     // 2KB (rarely used)
  KB_8 = 0x02,     // 8KB (1 bank)
  KB_32 = 0x03,    // 32KB (4 banks of 8KB each)
  KB_128 = 0x04,   // 128KB (16 banks of 8KB each)
  KB_64 = 0x05     // 64KB (8 banks of 8KB each)
};

class CartridgeHeader {
private:
  std::array<std::uint8_t, 0x50> header_data_{};

public:
  constexpr CartridgeHeader() = default;
  
  constexpr void parse_data(const std::uint8_t* data) {
    for (std::size_t i = 0; i < 0x50; ++i) {
      header_data_[i] = data[i + 0x100]; // Header starts at 0x100 in ROM
    }
  }
  
  [[nodiscard]] constexpr auto get_title() const -> std::string {
    // Title is at 0x134-0x143, but we'll just use up to 0x143 for simplicity
    std::string title;
    for (std::size_t i = 0x34; i <= 0x43; ++i) {
      const auto c = static_cast<char>(header_data_[i]);
      if (c == 0) break;
      title.push_back(c);
    }
    return title;
  }
  
  [[nodiscard]] constexpr auto get_cartridge_type() const -> CartridgeType {
    return static_cast<CartridgeType>(header_data_[0x47]);
  }
  
  [[nodiscard]] constexpr auto get_rom_size() const -> ROMSize {
    return static_cast<ROMSize>(header_data_[0x48]);
  }
  
  [[nodiscard]] constexpr auto get_ram_size() const -> RAMSize {
    return static_cast<RAMSize>(header_data_[0x49]);
  }
  
  [[nodiscard]] constexpr auto get_rom_banks() const -> std::uint32_t {
    const auto size = get_rom_size();
    switch (size) {
      case ROMSize::KB_32: return 2;   // 2 banks of 16KB
      case ROMSize::KB_64: return 4;   // 4 banks of 16KB
      case ROMSize::KB_128: return 8;  // 8 banks of 16KB
      case ROMSize::KB_256: return 16; // 16 banks of 16KB
      case ROMSize::KB_512: return 32; // 32 banks of 16KB
      case ROMSize::MB_1: return 64;   // 64 banks of 16KB
      case ROMSize::MB_2: return 128;  // 128 banks of 16KB
      case ROMSize::MB_4: return 256;  // 256 banks of 16KB
      case ROMSize::MB_8: return 512;  // 512 banks of 16KB
      default: return 2;               // Default to 2 banks
    }
  }
  
  [[nodiscard]] constexpr auto get_ram_banks() const -> std::uint32_t {
    const auto size = get_ram_size();
    switch (size) {
      case RAMSize::NONE: return 0;   // No RAM
      case RAMSize::KB_2: return 1;   // Just 1 tiny bank
      case RAMSize::KB_8: return 1;   // 1 bank of 8KB
      case RAMSize::KB_32: return 4;  // 4 banks of 8KB
      case RAMSize::KB_128: return 16; // 16 banks of 8KB
      case RAMSize::KB_64: return 8;  // 8 banks of 8KB
      default: return 0;              // Default to no RAM
    }
  }
  
  [[nodiscard]] constexpr auto is_japanese() const -> bool {
    return header_data_[0x4A] == 0x00;
  }
  
  [[nodiscard]] constexpr auto get_global_checksum() const -> std::uint16_t {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(header_data_[0x4E]) << 8U) | header_data_[0x4F]);
  }
  
  [[nodiscard]] constexpr auto validate_header_checksum() const -> bool {
    std::uint8_t checksum = 0;
    for (std::size_t i = 0x34; i <= 0x4C; ++i) {
      checksum = static_cast<std::uint8_t>(checksum - header_data_[i] - 1);
    }
    return checksum == header_data_[0x4D];
  }
};

} // namespace cycle_perfect_gameboy::cartridge

#endif // CYCLE_PERFECT_GAMEBOY_CARTRIDGE_HEADER_HPP