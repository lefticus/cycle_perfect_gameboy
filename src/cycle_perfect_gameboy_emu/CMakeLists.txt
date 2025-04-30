add_executable(cycle_perfect_gameboy_emu main.cpp)

target_link_libraries(cycle_perfect_gameboy_emu 
  PRIVATE 
    cycle_perfect_gameboy::cycle_perfect_gameboy
    cycle_perfect_gameboy_options
    cycle_perfect_gameboy_warnings
)

target_include_directories(cycle_perfect_gameboy_emu 
  ${WARNING_GUARD} 
  PRIVATE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)

target_compile_features(cycle_perfect_gameboy_emu PRIVATE cxx_std_23)

# Install the executable
install(TARGETS cycle_perfect_gameboy_emu
  RUNTIME DESTINATION bin
)