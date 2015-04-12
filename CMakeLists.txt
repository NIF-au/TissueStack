#
# Local config flag
#
set(${PROJECT_NAME}_LOCAL_CONFIG OFF)

include(CvlSetVersion)

if (${PROJECT_NAME}_LOCAL_CONFIG)
    #
    # Local configuration, using "echo" to escape a command.
    #
    # Using "echo" to escape a command.
    #
    # Download command
    set(${PROJECT_NAME}_DOWNLOAD_COMMAND echo)

    # Configure command
    set(${PROJECT_NAME}_CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/config <SOURCE_DIR> <INSTALL_DIR>)

    # Build command
    set(${PROJECT_NAME}_BUILD_COMMAND make)

    # Install command
    set(${PROJECT_NAME}_INSTALL_COMMAND echo)

endif(${PROJECT_NAME}_LOCAL_CONFIG)

include(CvlBuildConfig)
