# MenuConfig CMake Module
# This file provides menuconfig functionality for ZShell
# Include this file in your main CMakeLists.txt with: include(menuconfig.cmake)

# Configuration files
set(KCONFIG_CONFIG "${CMAKE_SOURCE_DIR}/.config")
set(CONFIG_HEADER "${CMAKE_SOURCE_DIR}/src/config.h")
set(GENERATE_CONFIG_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/generate_config.py")

# Check for required tools
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Check if Python menuconfig module is available
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import menuconfig"
    RESULT_VARIABLE MENUCONFIG_AVAILABLE
    OUTPUT_QUIET
    ERROR_QUIET
)

# Function to add menuconfig support to a target
function(add_menuconfig_support TARGET_NAME)
    # Make sure the target depends on config.h
    add_dependencies(${TARGET_NAME} generate_config)

    # Add include directory for config.h
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endfunction()

# Generate config.h from .config
add_custom_command(
    OUTPUT ${CONFIG_HEADER}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
    COMMAND ${Python3_EXECUTABLE} ${GENERATE_CONFIG_SCRIPT} ${KCONFIG_CONFIG} ${CONFIG_HEADER}
    DEPENDS ${KCONFIG_CONFIG} ${GENERATE_CONFIG_SCRIPT}
    COMMENT "Generating config.h from .config"
    VERBATIM
)

# Create .config with defaults if it doesn't exist
add_custom_command(
    OUTPUT ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "Creating default configuration..."
    COMMAND ${CMAKE_COMMAND} -E echo "# Automatically generated file; DO NOT EDIT." > ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_COMMAND_MAX=256" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_MAX_PATHS=8" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_HISTORY_MAX_ENTRIES=10" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_HISTORY_ENABLED=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_AUTOEXEC_ENABLED=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_COLOR_SUPPORT=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_PROMPT_CUSTOMIZATION=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_BUILTIN_COMMANDS=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_VERBOSE_ERRORS=y" >> ${KCONFIG_CONFIG}
    COMMENT "Creating default .config"
    VERBATIM
)

# Custom target to ensure config.h is generated
add_custom_target(generate_config DEPENDS ${CONFIG_HEADER})

# Menuconfig target (using Python menuconfig)
if(MENUCONFIG_AVAILABLE EQUAL 0)
    add_custom_target(menuconfig
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E env 
            MENUCONFIG_STYLE=aquatic 
            KCONFIG_CONFIG=${KCONFIG_CONFIG}
            ${Python3_EXECUTABLE} -m menuconfig ${CMAKE_SOURCE_DIR}/Kconfig
        COMMAND ${Python3_EXECUTABLE} ${GENERATE_CONFIG_SCRIPT} ${KCONFIG_CONFIG} ${CONFIG_HEADER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Python menuconfig"
        VERBATIM
    )
else()
    add_custom_target(menuconfig
        COMMAND ${CMAKE_COMMAND} -E echo "Error: Python menuconfig module not found. Install with: pip install menuconfig"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

# Config target (text-based using Python)
if(MENUCONFIG_AVAILABLE EQUAL 0)
    add_custom_target(config
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E env 
            KCONFIG_CONFIG=${KCONFIG_CONFIG}
            ${Python3_EXECUTABLE} -c "
import sys
try:
    import menuconfig
    sys.argv = ['menuconfig', '${CMAKE_SOURCE_DIR}/Kconfig']
    menuconfig.main()
except ImportError:
    print('Python menuconfig module not available')
    sys.exit(1)
"
        COMMAND ${Python3_EXECUTABLE} ${GENERATE_CONFIG_SCRIPT} ${KCONFIG_CONFIG} ${CONFIG_HEADER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running text-based config"
        VERBATIM
    )
else()
    add_custom_target(config
        COMMAND ${CMAKE_COMMAND} -E echo "Error: Python menuconfig module not found. Install with: pip install menuconfig"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

# Oldconfig target
if(MENUCONFIG_AVAILABLE EQUAL 0)
    add_custom_target(oldconfig
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E env 
            MENUCONFIG_STYLE=aquatic 
            KCONFIG_CONFIG=${KCONFIG_CONFIG}
            ${Python3_EXECUTABLE} -m menuconfig --oldconfig ${CMAKE_SOURCE_DIR}/Kconfig
        COMMAND ${Python3_EXECUTABLE} ${GENERATE_CONFIG_SCRIPT} ${KCONFIG_CONFIG} ${CONFIG_HEADER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Updating configuration"
        VERBATIM
    )
else()
    add_custom_target(oldconfig
        COMMAND ${CMAKE_COMMAND} -E echo "Error: Python menuconfig module not found. Install with: pip install menuconfig"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif()

# Defconfig target
add_custom_target(defconfig
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E echo "Creating default configuration..."
    COMMAND ${CMAKE_COMMAND} -E echo "# Automatically generated file; DO NOT EDIT." > ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_COMMAND_MAX=256" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_MAX_PATHS=8" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_HISTORY_MAX_ENTRIES=10" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_HISTORY_ENABLED=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_AUTOEXEC_ENABLED=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_COLOR_SUPPORT=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_PROMPT_CUSTOMIZATION=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_BUILTIN_COMMANDS=y" >> ${KCONFIG_CONFIG}
    COMMAND ${CMAKE_COMMAND} -E echo "CONFIG_VERBOSE_ERRORS=y" >> ${KCONFIG_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${GENERATE_CONFIG_SCRIPT} ${KCONFIG_CONFIG} ${CONFIG_HEADER}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Creating default configuration"
    VERBATIM
)

# Clean config target
add_custom_target(config_clean
    COMMAND ${CMAKE_COMMAND} -E remove -f ${KCONFIG_CONFIG} ${KCONFIG_CONFIG}.old ${CONFIG_HEADER}
    COMMENT "Removing configuration files"
)

# Add menuconfig help to existing help target (if it exists)
if(TARGET zshell_help)
    # Extend existing help
    add_custom_target(menuconfig_help
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Menuconfig targets:"
        COMMAND ${CMAKE_COMMAND} -E echo "  menuconfig  - Interactive configuration menu"
        COMMAND ${CMAKE_COMMAND} -E echo "  config      - Text-based configuration"
        COMMAND ${CMAKE_COMMAND} -E echo "  oldconfig   - Update config with new options"
        COMMAND ${CMAKE_COMMAND} -E echo "  defconfig   - Create default configuration"
        COMMAND ${CMAKE_COMMAND} -E echo "  config_clean - Remove configuration files"
        COMMAND ${CMAKE_COMMAND} -E echo ""
    )
    add_dependencies(zshell_help menuconfig_help)
else()
    # Create new help target
    add_custom_target(zshell_help
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Available targets:"
        COMMAND ${CMAKE_COMMAND} -E echo "  zshell       - Build the shell"
        COMMAND ${CMAKE_COMMAND} -E echo "  menuconfig   - Interactive configuration menu"
        COMMAND ${CMAKE_COMMAND} -E echo "  config       - Text-based configuration"
        COMMAND ${CMAKE_COMMAND} -E echo "  oldconfig    - Update config with new options"
        COMMAND ${CMAKE_COMMAND} -E echo "  defconfig    - Create default configuration"
        COMMAND ${CMAKE_COMMAND} -E echo "  config_clean - Remove configuration files"
        COMMAND ${CMAKE_COMMAND} -E echo "  help         - Show this help"
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Usage:"
        COMMAND ${CMAKE_COMMAND} -E echo "  cmake -B build"
        COMMAND ${CMAKE_COMMAND} -E echo "  cmake --build build --target menuconfig"
        COMMAND ${CMAKE_COMMAND} -E echo "  cmake --build build"
        COMMAND ${CMAKE_COMMAND} -E echo ""
    )
endif()

# Print menuconfig configuration info (only when not in a subdirectory)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "Menuconfig Configuration")
    message(STATUS "========================")
    message(STATUS "Python3: ${Python3_EXECUTABLE}")
    if(MENUCONFIG_AVAILABLE EQUAL 0)
        message(STATUS "Python menuconfig: FOUND")
    else()
        message(STATUS "Python menuconfig: NOT FOUND (install with: pip install menuconfig)")
    endif()
    message(STATUS "Config file: ${KCONFIG_CONFIG}")
    message(STATUS "Config header: ${CONFIG_HEADER}")
    message(STATUS "========================")
endif()