cmake_minimum_required(VERSION 3.16)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

zos_use(zvb)
find_package(ZVB_SDK REQUIRED)

# When using CMake as a build system, it's possible to mix ASM and C in the compilation
project(zshell C ASM)

# Include menuconfig support (optional)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/menuconfig.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/menuconfig.cmake)
endif()

add_executable(zshell
    "src/main.c"
    "src/common.c"
    "src/history.c"
    "src/paths.c"
    "src/batch.c"
    "src/process.c"
)

target_include_directories(zshell PRIVATE $ENV{ZVB_SDK_PATH}/include)
target_include_directories(zshell PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

# Add library directory before linking
target_link_directories(zshell PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib)

# Link with .lib extension for SDCC
target_link_libraries(zshell zvb keyboard.lib ansi.lib)

# Add menuconfig support to the zshell target (if available)
if(COMMAND add_menuconfig_support)
    add_menuconfig_support(zshell)
endif()

zos_add_outputs(zshell)

add_custom_command(TARGET zshell_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zshell.bin
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zshell

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/autoexec.zs
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/autoexec.zs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/test.zs
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test.zs
)