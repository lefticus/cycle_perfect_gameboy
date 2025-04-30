include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(cycle_perfect_gameboy_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(cycle_perfect_gameboy_setup_options)
  option(cycle_perfect_gameboy_ENABLE_HARDENING "Enable hardening" ON)
  option(cycle_perfect_gameboy_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    cycle_perfect_gameboy_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    cycle_perfect_gameboy_ENABLE_HARDENING
    OFF)

  cycle_perfect_gameboy_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR cycle_perfect_gameboy_PACKAGING_MAINTAINER_MODE)
    option(cycle_perfect_gameboy_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(cycle_perfect_gameboy_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(cycle_perfect_gameboy_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(cycle_perfect_gameboy_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(cycle_perfect_gameboy_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(cycle_perfect_gameboy_ENABLE_PCH "Enable precompiled headers" OFF)
    option(cycle_perfect_gameboy_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(cycle_perfect_gameboy_ENABLE_IPO "Enable IPO/LTO" ON)
    option(cycle_perfect_gameboy_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(cycle_perfect_gameboy_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(cycle_perfect_gameboy_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(cycle_perfect_gameboy_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(cycle_perfect_gameboy_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(cycle_perfect_gameboy_ENABLE_PCH "Enable precompiled headers" OFF)
    option(cycle_perfect_gameboy_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      cycle_perfect_gameboy_ENABLE_IPO
      cycle_perfect_gameboy_WARNINGS_AS_ERRORS
      cycle_perfect_gameboy_ENABLE_USER_LINKER
      cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS
      cycle_perfect_gameboy_ENABLE_SANITIZER_LEAK
      cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED
      cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD
      cycle_perfect_gameboy_ENABLE_SANITIZER_MEMORY
      cycle_perfect_gameboy_ENABLE_UNITY_BUILD
      cycle_perfect_gameboy_ENABLE_CLANG_TIDY
      cycle_perfect_gameboy_ENABLE_CPPCHECK
      cycle_perfect_gameboy_ENABLE_COVERAGE
      cycle_perfect_gameboy_ENABLE_PCH
      cycle_perfect_gameboy_ENABLE_CACHE)
  endif()

  cycle_perfect_gameboy_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS OR cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD OR cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(cycle_perfect_gameboy_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(cycle_perfect_gameboy_global_options)
  if(cycle_perfect_gameboy_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    cycle_perfect_gameboy_enable_ipo()
  endif()

  cycle_perfect_gameboy_supports_sanitizers()

  if(cycle_perfect_gameboy_ENABLE_HARDENING AND cycle_perfect_gameboy_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${cycle_perfect_gameboy_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED}")
    cycle_perfect_gameboy_enable_hardening(cycle_perfect_gameboy_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(cycle_perfect_gameboy_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(cycle_perfect_gameboy_warnings INTERFACE)
  add_library(cycle_perfect_gameboy_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  cycle_perfect_gameboy_set_project_warnings(
    cycle_perfect_gameboy_warnings
    ${cycle_perfect_gameboy_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(cycle_perfect_gameboy_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    cycle_perfect_gameboy_configure_linker(cycle_perfect_gameboy_options)
  endif()

  include(cmake/Sanitizers.cmake)
  cycle_perfect_gameboy_enable_sanitizers(
    cycle_perfect_gameboy_options
    ${cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS}
    ${cycle_perfect_gameboy_ENABLE_SANITIZER_LEAK}
    ${cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED}
    ${cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD}
    ${cycle_perfect_gameboy_ENABLE_SANITIZER_MEMORY})

  set_target_properties(cycle_perfect_gameboy_options PROPERTIES UNITY_BUILD ${cycle_perfect_gameboy_ENABLE_UNITY_BUILD})

  if(cycle_perfect_gameboy_ENABLE_PCH)
    target_precompile_headers(
      cycle_perfect_gameboy_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(cycle_perfect_gameboy_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    cycle_perfect_gameboy_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(cycle_perfect_gameboy_ENABLE_CLANG_TIDY)
    cycle_perfect_gameboy_enable_clang_tidy(cycle_perfect_gameboy_options ${cycle_perfect_gameboy_WARNINGS_AS_ERRORS})
  endif()

  if(cycle_perfect_gameboy_ENABLE_CPPCHECK)
    cycle_perfect_gameboy_enable_cppcheck(${cycle_perfect_gameboy_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(cycle_perfect_gameboy_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    cycle_perfect_gameboy_enable_coverage(cycle_perfect_gameboy_options)
  endif()

  if(cycle_perfect_gameboy_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(cycle_perfect_gameboy_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(cycle_perfect_gameboy_ENABLE_HARDENING AND NOT cycle_perfect_gameboy_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_UNDEFINED
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_ADDRESS
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_THREAD
       OR cycle_perfect_gameboy_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    cycle_perfect_gameboy_enable_hardening(cycle_perfect_gameboy_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
