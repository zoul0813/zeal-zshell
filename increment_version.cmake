# Read or create build number file
if(EXISTS ${BUILD_NUMBER_FILE})
    file(READ ${BUILD_NUMBER_FILE} ZSHELL_VERSION_BUILD)
    string(STRIP "${ZSHELL_VERSION_BUILD}" ZSHELL_VERSION_BUILD)
    math(EXPR ZSHELL_VERSION_BUILD "${ZSHELL_VERSION_BUILD} + 1")
else()
    set(ZSHELL_VERSION_BUILD 1)
endif()

# Write updated build number
file(WRITE ${BUILD_NUMBER_FILE} ${ZSHELL_VERSION_BUILD})

# Generate version.h from template
configure_file(
    ${VERSION_TEMPLATE}
    ${VERSION_OUTPUT}
    @ONLY
)

message(STATUS "Build number incremented to: ${ZSHELL_VERSION_BUILD}")