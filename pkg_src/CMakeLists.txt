cmake_minimum_required(VERSION 3.10)
project(sysmon C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler flags
add_compile_options(-Wall -Wextra)
# Disable pedantic errors for now
#add_compile_options(-pedantic -Werror)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Define source files
set(SOURCES
    sysmon.c
    config.c
    resources.c
    util.c
    json_handler.c
    cJSON.c
)

# Define headers files
set(HEADERS
    sysmon.h
    config.h
    resources.h
    util.h
    json_handler.h
    cJSON.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link with required libraries
target_link_libraries(${PROJECT_NAME} m)

# For getopt() - include unistd.h
target_compile_definitions(${PROJECT_NAME} PRIVATE _POSIX_C_SOURCE=200809L)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES default_config.json DESTINATION etc RENAME sysmon_config.json)