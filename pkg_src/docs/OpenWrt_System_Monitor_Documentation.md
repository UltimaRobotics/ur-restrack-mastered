# OpenWrt System Monitoring Application Documentation

## Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Build System](#build-system)
4. [Configuration](#configuration)
5. [Module Descriptions](#module-descriptions)
   - [Core Application (sysmon.c/h)](#core-application)
   - [Configuration Management (config.c/h)](#configuration-management)
   - [Resource Collection (resources.c/h)](#resource-collection)
   - [JSON Handling (json_handler.c/h)](#json-handling)
   - [Utilities (util.c/h)](#utilities)
   - [JSON Library (cJSON.c/h)](#json-library)
6. [Data Structure](#data-structure)
7. [Error Handling](#error-handling)
8. [Command-Line Interface](#command-line-interface)
9. [Installation](#installation)
10. [Run Modes](#run-modes)
11. [Extending the Application](#extending-the-application)

## Overview <a name="overview"></a>

The OpenWrt System Monitoring Application (`sysmon`) is a C program designed to collect, log, and store system resource information from OpenWrt-based systems. The application captures various metrics such as CPU usage, memory usage, system load, disk usage, network statistics, uptime, process information, and swap usage. 

The collected data is stored in a structured JSON format with timestamp information, which allows for historical tracking of system performance. The application supports customization via a configuration file that lets users determine which resources to monitor and how frequently to collect data.

## Architecture <a name="architecture"></a>

The application follows a modular design with clear separation of concerns:

```
+-------------------+        +-------------------+
|  Core Application |------->| Config Management |
| (sysmon.c/h)      |        | (config.c/h)      |
+-------------------+        +-------------------+
        |
        |                    +-------------------+
        +----------------+-->| Resource Collection|
        |                    | (resources.c/h)    |
        |                    +-------------------+
        |
        |                    +-------------------+
        +----------------+-->| JSON Handling     |
        |                    | (json_handler.c/h)|
        |                    +-------------------+
        |
        |                    +-------------------+
        +----------------+-->| Utilities         |
                             | (util.c/h)        |
                             +-------------------+
```

- **Core Application**: Handles program initialization, command-line argument processing, and the main monitoring loop.
- **Configuration Management**: Manages loading, saving, and applying configuration settings.
- **Resource Collection**: Collects system resource metrics from various sources.
- **JSON Handling**: Manages creation, updating, and manipulation of JSON data structures.
- **Utilities**: Provides common functionality like logging, file I/O, and timestamp generation.
- **JSON Library**: External library (cJSON) for JSON parsing and generation.

## Build System <a name="build-system"></a>

The application uses CMake as its build system. The configuration is defined in `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(sysmon C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler flags
add_compile_options(-Wall -Wextra)

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
```

In addition, a shell script, `build.sh`, automates the build process and ensures that the required cJSON library files are available:

```bash
#!/bin/bash
set -e

echo "=== Downloading cJSON files if needed ==="
# Download cJSON files directly if they don't exist
if [ ! -f "cJSON.h" ]; then
    echo "Downloading cJSON.h..."
    curl -s -o cJSON.h https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.h
fi

if [ ! -f "cJSON.c" ]; then
    echo "Downloading cJSON.c..."
    curl -s -o cJSON.c https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.c
fi

echo "=== Creating build directory ==="
# Create build directory if it doesn't exist
mkdir -p build

# Change to build directory
cd build

echo "=== Running CMake ==="
# Run CMake
cmake ..

echo "=== Building the project ==="
# Build the project
make VERBOSE=1

# If the build succeeds, copy the executable to the root directory
if [ -f "sysmon" ]; then
    cp sysmon ..
    echo "=== Build completed successfully ==="
    echo "Executable 'sysmon' created."
else
    echo "=== Build failed ==="
    exit 1
fi
```

To build the application:
1. Make the build script executable: `chmod +x build.sh`
2. Run the build script: `./build.sh`

## Configuration <a name="configuration"></a>

### Configuration Structure

The application uses a configuration structure (`SysmonConfig`) defined in `sysmon.h`:

```c
typedef struct {
    char output_path[256];       // Path to output JSON file
    char log_path[256];          // Path to log file
    int collection_interval;     // Collection interval in seconds
    int verbose;                 // Verbose output flag
    
    // Resource collection flags
    int collect_cpu;             // Collect CPU usage
    int collect_memory;          // Collect memory usage
    int collect_load;            // Collect system load
    int collect_disk;            // Collect disk usage/IO
    int collect_network;         // Collect network statistics
    int collect_uptime;          // Collect system uptime
    int collect_processes;       // Collect process information
    int collect_swap;            // Collect swap usage
} SysmonConfig;
```

### Default Configuration File (default_config.json)

```json
{
  "output_path": "/var/log/sysmon_data.json",
  "log_path": "/var/log/sysmon.log",
  "collection_interval": 60,
  "verbose": false,
  "collect_cpu": true,
  "collect_memory": true,
  "collect_load": true,
  "collect_disk": true,
  "collect_network": true,
  "collect_uptime": true,
  "collect_processes": true,
  "collect_swap": true
}
```

### Configuration Handling

The application loads configuration from a JSON file and applies defaults where necessary. Command-line arguments can override configuration file settings. Configuration management is handled in `config.c/h`.

## Module Descriptions <a name="module-descriptions"></a>

### Core Application <a name="core-application"></a>

**Files**: `sysmon.c`, `sysmon.h`

The core application module serves as the entry point and main controller for the system monitoring application. It:

- Processes command-line arguments
- Initializes the logging system
- Loads the configuration
- Manages the main monitoring loop
- Collects system resources
- Updates the JSON data file
- Handles the run-once option for one-time execution

Key functions:
- `main()`: Entry point for the application.
- `print_usage()`: Displays command-line usage information.

### Configuration Management <a name="configuration-management"></a>

**Files**: `config.c`, `config.h`

The configuration management module handles loading, saving, and managing configuration settings. It:

- Loads configuration from JSON files
- Applies default settings when needed
- Saves configuration changes
- Displays configuration information

Key functions:
- `set_default_config()`: Sets default configuration values.
- `load_config()`: Loads configuration from a JSON file.
- `save_config()`: Saves configuration to a JSON file.
- `print_config()`: Prints configuration values.

### Resource Collection <a name="resource-collection"></a>

**Files**: `resources.c`, `resources.h`

The resource collection module is responsible for gathering system metrics. It:

- Collects various system resources based on configuration flags
- Creates JSON representations of the collected data
- Handles resource collection errors

Key functions:
- `collect_all_resources()`: Main function that collects all configured resources.
- Resource-specific collection functions:
  - `collect_cpu_usage()`
  - `collect_memory_usage()`
  - `collect_system_load()`
  - `collect_disk_usage()`
  - `collect_network_stats()`
  - `collect_system_uptime()`
  - `collect_process_info()`
  - `collect_swap_usage()`

### JSON Handling <a name="json-handling"></a>

**Files**: `json_handler.c`, `json_handler.h`

The JSON handling module manages operations on JSON data structures. It:

- Updates JSON files with new data
- Merges JSON objects
- Creates history entries in the JSON data structure

Key functions:
- `update_json_file()`: Updates a JSON file with new data.
- `merge_json_objects()`: Merges two JSON objects.
- `create_history_entry()`: Creates a history entry for a JSON object.

### Utilities <a name="utilities"></a>

**Files**: `util.c`, `util.h`

The utilities module provides common functionality used throughout the application. It:

- Manages the logging system
- Handles file I/O operations
- Generates and formats timestamps
- Adds timestamps to JSON objects

Key functions:
- `init_logger()`: Initializes the logging system.
- `log_message()`: Logs a message with a specified level.
- `read_file()`: Reads the contents of a file into a string.
- `write_file()`: Writes a string to a file.
- `add_timestamp()`: Adds a timestamp to a JSON object.
- `get_timestamp()`: Generates a formatted timestamp string.

### JSON Library <a name="json-library"></a>

**Files**: `cJSON.c`, `cJSON.h`

The application uses the cJSON library for JSON parsing and generation. This is an external dependency that is automatically downloaded by the build script if not present.

## Data Structure <a name="data-structure"></a>

The application stores collected data in a structured JSON format. The main structure is:

```json
{
  "history": [
    {
      "timestamp": "YYYY-MM-DD HH:MM:SS",
      "cpu_usage": { ... },
      "memory": { ... },
      "load": { ... },
      "disk": { ... },
      "network": { ... },
      "uptime": { ... },
      "processes": { ... },
      "swap": { ... }
    },
    { ... }  // Additional history entries
  ]
}
```

Each resource type has its own structure within the history entry:

- **CPU Usage**:
  ```json
  "cpu_usage": {
    "cpu_count": 8,
    "cpus": [
      {
        "name": "cpu0",
        "user": 1286155,
        "nice": 184,
        "system": 329367,
        "idle": 2994402,
        "iowait": 67979,
        "usage_percent": 38.5360110300117
      },
      { ... }  // Additional CPUs
    ]
  }
  ```

- **Memory**:
  ```json
  "memory": {
    "total_memory": 8589934592,  // in bytes
    "free_memory": 3221225472,   // in bytes
    "shared_memory": 1073741824, // in bytes
    "buffer_memory": 536870912,  // in bytes
    "usage_percent": 62.5
  }
  ```

- **System Load**:
  ```json
  "load": {
    "load_1min": 0.42,
    "load_5min": 0.51,
    "load_15min": 0.62
  }
  ```

- **Disk**:
  ```json
  "disk": {
    "filesystems": [
      {
        "mount_point": "/",
        "size": 107374182400,  // in bytes
        "used": 42949672960,   // in bytes
        "available": 64424509440, // in bytes
        "usage_percent": 40.0
      },
      { ... }  // Additional filesystems
    ]
  }
  ```

- **Network**:
  ```json
  "network": {
    "interfaces": [
      {
        "name": "eth0",
        "rx_bytes": 1073741824,  // in bytes
        "tx_bytes": 536870912,   // in bytes
        "rx_packets": 1000000,
        "tx_packets": 500000,
        "rx_errors": 0,
        "tx_errors": 0
      },
      { ... }  // Additional interfaces
    ]
  }
  ```

- **Uptime**:
  ```json
  "uptime": {
    "uptime_seconds": 2592000,  // 30 days in seconds
    "uptime_pretty": "30 days, 0 hours, 0 minutes"
  }
  ```

- **Processes**:
  ```json
  "processes": {
    "total_processes": 350,
    "running_processes": 5,
    "blocked_processes": 0,
    "sleeping_processes": 345
  }
  ```

- **Swap**:
  ```json
  "swap": {
    "total_swap": 2147483648,  // in bytes
    "used_swap": 1073741824,   // in bytes
    "free_swap": 1073741824,   // in bytes
    "usage_percent": 50.0
  }
  ```

## Error Handling <a name="error-handling"></a>

The application uses predefined error codes for consistent error handling:

```c
// Error codes
#define ERR_SUCCESS 0
#define ERR_FILE_OPEN -1
#define ERR_FILE_READ -2
#define ERR_FILE_WRITE -3
#define ERR_JSON_PARSE -4
#define ERR_JSON_CREATE -5
#define ERR_SYS_RESOURCE -6
#define ERR_MEMORY_ALLOC -7
#define ERR_CONFIG_MISSING -8
#define ERR_INVALID_PARAM -9
```

Errors are logged using the logging system with appropriate severity levels:

```c
// Log levels
#define LOG_ERROR 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3
```

Example error handling:
```c
int result = update_json_file(g_config.output_path, resource_data);
if (result != ERR_SUCCESS) {
    log_message(LOG_ERROR, "Failed to update JSON file: %d", result);
} else {
    log_message(LOG_INFO, "Successfully updated system resource data");
}
```

## Command-Line Interface <a name="command-line-interface"></a>

The application supports various command-line options:

```
Usage: sysmon [options]
Options:
  -c <config_file>   Specify configuration file (default: /etc/sysmon_config.json)
  -o <output_file>   Specify output file (default: from config or /var/log/sysmon_data.json)
  -l <log_file>      Specify log file (default: from config or /var/log/sysmon.log)
  -i <interval>      Specify collection interval in seconds (default: from config or 60)
  -v                 Enable verbose output
  -x                 Run once and exit
  -h                 Display this help message
```

Example usage:
- Run with default settings: `./sysmon`
- Specify config file: `./sysmon -c /path/to/config.json`
- Run once with verbose output: `./sysmon -v -x`
- Specify output file: `./sysmon -o /path/to/output.json`

## Installation <a name="installation"></a>

To install the application system-wide (required root permissions):

1. Build the application: `./build.sh`
2. Install the application: `sudo make install`

This will install:
- The `sysmon` executable to `/usr/local/bin/`
- The default configuration file to `/etc/sysmon_config.json`

## Run Modes <a name="run-modes"></a>

The application can operate in two main modes:

1. **Continuous Monitoring** (default): The application runs in a loop, collecting data at the specified interval.
   
   Example: `./sysmon -i 300`  # Collect data every 5 minutes

2. **Run Once**: The application collects data once and then exits.
   
   Example: `./sysmon -x`  # Collect data once and exit

## Extending the Application <a name="extending-the-application"></a>

The modular design makes the application easy to extend:

### Adding a New Resource Type

1. Define the collection function in `resources.h`:
   ```c
   /**
    * @brief Collect new resource information
    * @return cJSON object with new resource data or NULL on failure
    */
   cJSON* collect_new_resource(void);
   ```

2. Implement the collection function in `resources.c`:
   ```c
   cJSON* collect_new_resource(void) {
       cJSON *new_resource = cJSON_CreateObject();
       if (new_resource == NULL) {
           log_message(LOG_ERROR, "Failed to create JSON object for new resource");
           return NULL;
       }
       
       // Collect the resource data
       // ...
       
       return new_resource;
   }
   ```

3. Update the `collect_all_resources()` function in `resources.c`:
   ```c
   // Add flag to configuration structure in sysmon.h
   typedef struct {
       // ...
       int collect_new_resource;  // Collect new resource
   } SysmonConfig;
   
   // Update collect_all_resources() in resources.c
   if (config->collect_new_resource) {
       cJSON *new_resource_data = collect_new_resource();
       if (new_resource_data != NULL) {
           cJSON_AddItemToObject(resources, "new_resource", new_resource_data);
       }
   }
   ```

4. Update the default configuration:
   - Add to `default_config.json`:
     ```json
     "collect_new_resource": true
     ```
   - Update `set_default_config()` in `config.c`:
     ```c
     config->collect_new_resource = 1;
     ```
   - Update `load_config()` in `config.c` to parse the new flag.

With these changes, the application will collect and store the new resource type when configured to do so.