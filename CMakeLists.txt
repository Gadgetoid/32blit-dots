# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.9)

# Wrapper for the Pico SDK, which is only included if PICO_BOARD is set
include(${32BLIT_DIR}/32blit-pico/sdk_import.cmake)

project(dots)

set(PROJECT_SOURCE
  src/game.cpp
  src/main-menu.cpp
  src/menu.cpp
  src/random.cpp
  src/system.cpp
  src/gamemodes/gamemode.cpp
  src/gamemodes/normal.cpp
  src/gamemodes/puzzle.cpp)

set(PROJECT_DISTRIBS LICENSE README.md)

if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324" "/wd4458" "/wd4100")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion" "-Wno-unused-parameter")
endif()

find_package (32BLIT CONFIG REQUIRED PATHS ../32blit-sdk $ENV{PATH_32BLIT_SDK})

blit_executable (${PROJECT_NAME} ${PROJECT_SOURCE})
blit_assets_yaml (${PROJECT_NAME} assets.yml)
blit_metadata (${PROJECT_NAME} metadata.yml)

include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/src/gamemodes)
target_compile_definitions(${PROJECT_NAME} PRIVATE ALLOW_HIRES=1)
add_custom_target (flash DEPENDS ${PROJECT_NAME}.flash)

install (FILES ${PROJECT_DISTRIBS} DESTINATION .)
set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set (CPACK_GENERATOR "ZIP" "TGZ")
include (CPack)
