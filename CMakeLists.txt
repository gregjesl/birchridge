cmake_minimum_required (VERSION 3.11)

project(birchridge)

# Load macrothreading
if(NOT TARGET macrothreading)
    include(FetchContent)
    FetchContent_Declare(macrothreading_lib
        GIT_REPOSITORY https://github.com/gregjesl/macrothreading
    )
    FetchContent_Populate(macrothreading_lib)
    add_subdirectory(${macrothreading_lib_SOURCE_DIR})
endif()

# Load socketsession
if(NOT TARGET socket-sessions)
    include(FetchContent)
    FetchContent_Declare(socketsession_lib
        GIT_REPOSITORY https://github.com/gregjesl/socket-sessions
    )
    FetchContent_Populate(socketsession_lib)
    add_subdirectory(${socketsession_lib_SOURCE_DIR})
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
    # Create the library
    add_library(${PROJECT_NAME} ${sources})

    # Include the include directory
    target_include_directories(${PROJECT_NAME} PUBLIC inc)

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