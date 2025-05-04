cmake_minimum_required(VERSION 3.10)
project(sysmon C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -Wno-maybe-uninitialized -Wno-int-conversion -Wno-return-type -Wno-sign-compare -D_DEBUG -DWITH_THREADING")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-implicit-function-declaration -Wno-enum-conversion -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-function")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers -Wno-missing-braces -Wno-uninitialized")

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(thirdparty/ur-rpc-template)
add_subdirectory(thirdparty/ur-threadder-api)

add_library(ur-restrack-api STATIC spec/ur-restrack.c)
target_link_libraries(ur-restrack-api PRIVATE ur-rpc-template threadmanager pthread)
target_include_directories(ur-restrack-api PRIVATE src)
target_include_directories(ur-restrack-api PRIVATE thirdparty/ur-threadder-api/include)
target_include_directories(ur-restrack-api PRIVATE thirdparty/ur-rpc-template)

# Define source files
set(SOURCES
    src/sysmon.c
    src/config.c
    src/resources.c
    src/util.c
    src/json_handler.c
    src/cJSON.c
)

# Define headers files
set(HEADERS
    src/sysmon.h
    src/config.h
    src/resources.h
    src/util.h
    src/json_handler.h
    src/cJSON.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/ur-threadder-api/include)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/ur-rpc-template)
target_include_directories(${PROJECT_NAME} PRIVATE spec)

# Link with required libraries
target_link_libraries(${PROJECT_NAME} ur-restrack-api ur-rpc-template threadmanager pthread m)

# For getopt() - include unistd.h
target_compile_definitions(${PROJECT_NAME} PRIVATE _POSIX_C_SOURCE=200809L)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES default_config.json DESTINATION etc RENAME sysmon_config.json)
