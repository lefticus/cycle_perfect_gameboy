add_library(cycle_perfect_gameboy
  core/types.cpp
  memory/memory_map.cpp
  cpu/cpu.cpp
  cartridge/rom.cpp
  system/gameboy.cpp
  system/timer.cpp
  system/interrupt_controller.cpp
)

add_library(cycle_perfect_gameboy::cycle_perfect_gameboy ALIAS cycle_perfect_gameboy)

target_link_libraries(cycle_perfect_gameboy 
  PRIVATE cycle_perfect_gameboy_options 
  PRIVATE cycle_perfect_gameboy_warnings
)

target_include_directories(cycle_perfect_gameboy 
  ${WARNING_GUARD} 
  PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)

target_compile_features(cycle_perfect_gameboy PUBLIC cxx_std_23)

set_target_properties(
  cycle_perfect_gameboy
  PROPERTIES 
    VERSION ${PROJECT_VERSION}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# Install the library
install(TARGETS cycle_perfect_gameboy
  EXPORT cycle_perfect_gameboy-targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)