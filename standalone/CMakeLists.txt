cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(GreeterStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GIT_REPOSITORY https://hub.fastgit.org/jarro2783/cxxopts.git
  VERSION 2.2.1
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage(NAME Greeter SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(GreeterStandalone ${sources})

set_target_properties(GreeterStandalone PROPERTIES CXX_STANDARD 11 OUTPUT_NAME "Greeter")

target_link_libraries(GreeterStandalone Greeter::Greeter cxxopts ssl crypto hv ireader_media_server)
