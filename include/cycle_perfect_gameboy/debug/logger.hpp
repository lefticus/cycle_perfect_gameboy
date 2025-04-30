#ifndef CYCLE_PERFECT_GAMEBOY_DEBUG_LOGGER_HPP
#define CYCLE_PERFECT_GAMEBOY_DEBUG_LOGGER_HPP

#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "cycle_perfect_gameboy/core/types.hpp"
#include "cycle_perfect_gameboy/cpu/cpu.hpp"
#include "cycle_perfect_gameboy/cpu/registers.hpp"

namespace cycle_perfect_gameboy::debug {

enum class LogLevel {
  NONE,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  TRACE
};

class Logger {
private:
  static inline LogLevel current_level_ = LogLevel::INFO;
  static inline std::optional<std::ofstream> log_file_ = std::nullopt;
  static inline bool console_output_ = true;
  static inline std::uint64_t instruction_count_ = 0;

public:
  static void set_level(LogLevel level) {
    current_level_ = level;
  }

  static void set_log_file(const std::string& filename) {
    log_file_.emplace(filename);
  }

  static void set_console_output(bool enabled) {
    console_output_ = enabled;
  }

  static void close_log_file() {
    if (log_file_.has_value()) {
      log_file_->close();
      log_file_.reset();
    }
  }

  template <typename... Args>
  static void error(const std::string& format, Args&&... args) {
    if (current_level_ >= LogLevel::ERROR) {
      log("[ERROR] " + format, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void warning(const std::string& format, Args&&... args) {
    if (current_level_ >= LogLevel::WARNING) {
      log("[WARNING] " + format, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void info(const std::string& format, Args&&... args) {
    if (current_level_ >= LogLevel::INFO) {
      log("[INFO] " + format, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void debug(const std::string& format, Args&&... args) {
    if (current_level_ >= LogLevel::DEBUG) {
      log("[DEBUG] " + format, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  static void trace(const std::string& format, Args&&... args) {
    if (current_level_ >= LogLevel::TRACE) {
      log("[TRACE] " + format, std::forward<Args>(args)...);
    }
  }

  static void log_cpu_state(const cpu::CPU& cpu) {
    if (current_level_ >= LogLevel::TRACE) {
      const auto& regs = cpu.get_registers();

      std::stringstream ss;
      ss << "Instruction #" << instruction_count_++ << " | ";
      ss << "PC:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_pc().value();
      ss << " | AF:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_af().value();
      ss << " | BC:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_bc().value();
      ss << " | DE:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_de().value();
      ss << " | HL:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_hl().value();
      ss << " | SP:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_sp().value();
      
      log(ss.str());
    }
  }

  static void log_instruction(const cpu::CPU& cpu, std::uint8_t opcode, 
                              const std::array<std::uint8_t, 2>& operands, std::uint8_t operand_length) {
    if (current_level_ >= LogLevel::DEBUG) {
      const auto& regs = cpu.get_registers();
      
      std::stringstream ss;
      ss << "PC:" << std::hex << std::setw(4) << std::setfill('0') << regs.get_pc().value();
      ss << " | Op:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
      
      if (operand_length > 0) {
        ss << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(operands[0]);
      }
      if (operand_length > 1) {
        ss << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(operands[1]);
      }
      
      log(ss.str());
    }
  }

  static void log_memory_dump(const memory::MemoryInterface& memory, core::Address start, std::uint16_t length) {
    if (current_level_ >= LogLevel::DEBUG) {
      std::stringstream ss;
      ss << "Memory dump from " << std::hex << std::setw(4) << std::setfill('0') << start.value() 
         << " to " << std::hex << std::setw(4) << std::setfill('0') << (start.value() + length - 1) << ":\n";
      
      for (std::uint16_t i = 0; i < length; i += 16) {
        ss << std::hex << std::setw(4) << std::setfill('0') << (start.value() + i) << ": ";
        
        for (std::uint16_t j = 0; j < 16 && (i + j) < length; ++j) {
          const core::Address addr{static_cast<std::uint16_t>(start.value() + i + j)};
          ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(memory.read(addr)) << " ";
        }
        
        ss << "  ";
        
        for (std::uint16_t j = 0; j < 16 && (i + j) < length; ++j) {
          const core::Address addr{static_cast<std::uint16_t>(start.value() + i + j)};
          const char c = static_cast<char>(memory.read(addr));
          ss << (std::isprint(c) ? c : '.');
        }
        
        ss << "\n";
      }
      
      log(ss.str());
    }
  }

private:
  template <typename... Args>
  static void log(const std::string& format, Args&&... args) {
    const std::string message = format_string(format, std::forward<Args>(args)...);
    
    if (console_output_) {
      std::cout << message << std::endl;
    }
    
    if (log_file_.has_value()) {
      *log_file_ << message << std::endl;
    }
  }

  template <typename... Args>
  static std::string format_string(const std::string& format, Args&&... args) {
    // This is a simple placeholder. In a real implementation, we would use
    // std::format (C++20) or a custom implementation to format the string.
    // For now, we just return the format string.
    return format;
  }
};

} // namespace cycle_perfect_gameboy::debug

#endif // CYCLE_PERFECT_GAMEBOY_DEBUG_LOGGER_HPP