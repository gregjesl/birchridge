cmake_minimum_required (VERSION 3.8)

project(birchridge)

# Load socketsession
if(NOT TARGET socket-sessions)
    # Check to see if the folder is already cloned
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/socketsessions)
        execute_process(COMMAND git pull origin master
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/socketsessions
        )
    else()
        execute_process(COMMAND git clone https://github.com/gregjesl/socket-sessions lib/socketsessions
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()

    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/socketsessions")
endif()

# Get the source files
file(GLOB sources "src/*.c")

if(ESP_PLATFORM)
    if(__idf_component_context)
        idf_component_register(
            SRCS ${sources}
            INCLUDE_DIRS "inc"
        )
    else()
        set(IS_ESP_COMPONENT True)
    endif()
else()
    # Include directory
    include_directories(inc)

    # Create the library
    add_library(${PROJECT_NAME} ${sources})

    # Link the library
    if(NOT TARGET socket-sessions)
        message(FATAL_ERROR "No socket-sessions library found")
    endif()
    target_link_libraries (${PROJECT_NAME} PUBLIC socket-sessions)

    # Check for top-level project
    if(${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
        enable_testing()
        add_subdirectory(test)
    endif()

    # Make demo
    add_subdirectory(demo)
endif()