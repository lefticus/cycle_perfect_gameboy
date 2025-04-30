
message(${CMAKE_PREFIX_PATH})

# Generic test that uses conan libs
add_executable(intro main.cpp)
target_link_libraries(
  intro
  PRIVATE project_options
          project_warnings)

target_link_system_libraries(
  intro
  PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component)

set_target_properties(intro PROPERTIES SUFFIX ".html")
target_include_directories(intro PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
