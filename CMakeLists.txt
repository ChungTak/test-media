cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Greeter
  VERSION 1.0
  LANGUAGES C CXX
)
# https://stackoverflow.com/questions/38296756/what-is-the-idiomatic-way-in-cmake-to-add-the-fpic-compiler-option/64552011#64552011
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("https://hub.fastgit.org/TheLartians/PackageProject.cmake.git@1.6.0")

CPMAddPackage(
  NAME fmt
  GIT_TAG 7.1.3
  GIT_REPOSITORY "https://hub.fastgit.org/fmtlib/fmt.git"
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

CPMAddPackage(
        NAME openssl-cmake
        GIT_REPOSITORY https://hub.fastgit.org/janbar/openssl-cmake.git
        GIT_TAG master
        OPTIONS
        "WITH_APPS OFF"
)
if (openssl-cmake_ADDED)
    set(OPENSSL_INCLUDE_DIR "${openssl-cmake_BINARY_DIR}/include")
    target_include_directories(ssl INTERFACE ${OPENSSL_INCLUDE_DIR})
    target_include_directories(crypto INTERFACE ${OPENSSL_INCLUDE_DIR})
endif()

CPMAddPackage(
  NAME libhv
  GIT_REPOSITORY https://hub.fastgit.org/ithewei/libhv.git
  GIT_TAG master
  OPTIONS "BUILD_EXAMPLES OFF" "WITH_OPENSSL ON"
)
if(libhv_ADDED)
   target_include_directories(hv INTERFACE "${libhv_BINARY_DIR}/include")
   include_directories("${libhv_BINARY_DIR}/include")
endif()

CPMAddPackage(
  NAME ireader_sdk
  GIT_REPOSITORY https://hub.fastgit.org/ireader/sdk.git
  GIT_TAG master
  DOWNLOAD_ONLY True
)

CPMAddPackage(
  NAME ireader_media_server
  GIT_REPOSITORY https://hub.fastgit.org/ireader/media-server.git
  GIT_TAG master
  DOWNLOAD_ONLY True
)

if (ireader_media_server_ADDED)
  message(STATUS "ENABLE_ireader_media_server")
  # no CMake support, so we create our own target
  FILE(GLOB ireader_media_server_sources ${ireader_media_server_SOURCE_DIR}/libflv/source/*.c
                                         ${ireader_media_server_SOURCE_DIR}/librtmp/source/*.c)
  add_library(ireader_media_server STATIC ${ireader_media_server_sources})
  # add_definitions(-D_OPENSSL_) # librtmp ssl
  target_compile_definitions(ireader_media_server PRIVATE _OPENSSL_)

  include_directories(${ireader_media_server_SOURCE_DIR}/libflv/include)
  include_directories(${ireader_media_server_SOURCE_DIR}/librtmp/include)

  target_include_directories(ireader_media_server
    PUBLIC
      $<BUILD_INTERFACE:${ireader_media_server_SOURCE_DIR}/libflv/include>
      $<BUILD_INTERFACE:${ireader_media_server_SOURCE_DIR}/librtmp/include>
  )
  #install(TARGETS ireader_media_server DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib)


endif()

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(Greeter INTERFACE)
add_library(Greeter ${headers} ${sources})

set_target_properties(Greeter PROPERTIES CXX_STANDARD 11)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(Greeter PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

add_dependencies(Greeter  fmt::fmt ireader_media_server)
# Link dependencies
target_link_libraries(Greeter PRIVATE fmt::fmt)

target_include_directories(
  Greeter PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                 $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "fmt 7.1.3 ireader_media_server"
)
