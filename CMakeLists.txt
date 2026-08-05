###############################################################################
# Games in C by Professor Mario
###############################################################################
cmake_minimum_required(VERSION 3.26)


###############################################################################
# A Roguelike
###############################################################################
project(
  a_roguelike  
  LANGUAGES C
)


###############################################################################
# Dependencies built from source
###############################################################################
set(GINC_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty"
    CACHE PATH "Path to shared third-party sources"
)

get_filename_component(GINC_THIRDPARTY_DIR "${GINC_THIRDPARTY_DIR}" ABSOLUTE)

if(NOT EXISTS "${GINC_THIRDPARTY_DIR}/SDL/CMakeLists.txt")
  message(FATAL_ERROR "SDL3 source not found at ${GINC_THIRDPARTY_DIR}/SDL")
endif()

add_subdirectory(
  "${GINC_THIRDPARTY_DIR}/SDL"
  "${CMAKE_CURRENT_BINARY_DIR}/_deps/SDL"
  EXCLUDE_FROM_ALL
)


###############################################################################
# Project code
###############################################################################
add_subdirectory(src)
