# OpenWrt System Monitor API Reference

## Core API (sysmon.h)

### Data Structures

#### SysmonConfig
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

### Constants

#### Default Paths
```c
#define DEFAULT_CONFIG_PATH "/etc/sysmon_config.json"
#define DEFAULT_OUTPUT_PATH "/var/log/sysmon_data.json"
#define DEFAULT_LOG_PATH "/var/log/sysmon.log"
#define DEFAULT_COLLECTION_INTERVAL 60 // seconds
```

#### Error Codes
```c
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

#### Log Levels
```c
#define LOG_ERROR 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3
```

### Functions

#### `int main(int argc, char *argv[])`
Main entry point for the application.

**Parameters:**
- `argc`: Argument count
- `argv`: Argument values

**Returns:** 0 on successful execution, non-zero on error.

#### `void print_usage(char *progname)`
Prints usage information for the application.

**Parameters:**
- `progname`: Name of the program (typically argv[0])

**Returns:** None

## Configuration Management API (config.h)

### Functions

#### `void set_default_config(SysmonConfig *config)`
Sets default configuration values.

**Parameters:**
- `config`: Pointer to configuration structure to populate

**Returns:** None

#### `int load_config(const char *config_path, SysmonConfig *config)`
Loads configuration from a JSON file.

**Parameters:**
- `config_path`: Path to configuration file
- `config`: Pointer to configuration structure to populate

**Returns:** ERR_SUCCESS on success, error code on failure

#### `int save_config(const char *config_path, SysmonConfig *config)`
Saves configuration to a JSON file.

**Parameters:**
- `config_path`: Path to configuration file
- `config`: Pointer to configuration structure

**Returns:** ERR_SUCCESS on success, error code on failure

#### `void print_config(SysmonConfig *config)`
Prints configuration values.

**Parameters:**
- `config`: Pointer to configuration structure

**Returns:** None

## Resource Collection API (resources.h)

### Functions

#### `cJSON* collect_all_resources(SysmonConfig *config)`
Collects all system resources based on configuration.

**Parameters:**
- `config`: Pointer to configuration structure

**Returns:** cJSON object with collected resources or NULL on failure

#### `cJSON* collect_cpu_usage(void)`
Collects CPU usage information.

**Parameters:** None

**Returns:** cJSON object with CPU usage data or NULL on failure

#### `cJSON* collect_memory_usage(void)`
Collects memory usage information.

**Parameters:** None

**Returns:** cJSON object with memory usage data or NULL on failure

#### `cJSON* collect_system_load(void)`
Collects system load information.

**Parameters:** None

**Returns:** cJSON object with system load data or NULL on failure

#### `cJSON* collect_disk_usage(void)`
Collects disk usage and IO information.

**Parameters:** None

**Returns:** cJSON object with disk usage data or NULL on failure

#### `cJSON* collect_network_stats(void)`
Collects network statistics.

**Parameters:** None

**Returns:** cJSON object with network statistics or NULL on failure

#### `cJSON* collect_system_uptime(void)`
Collects system uptime information.

**Parameters:** None

**Returns:** cJSON object with uptime data or NULL on failure

#### `cJSON* collect_process_info(void)`
Collects process information.

**Parameters:** None

**Returns:** cJSON object with process information or NULL on failure

#### `cJSON* collect_swap_usage(void)`
Collects swap usage information.

**Parameters:** None

**Returns:** cJSON object with swap usage data or NULL on failure

## JSON Handling API (json_handler.h)

### Functions

#### `int update_json_file(const char *file_path, cJSON *new_data)`
Updates a JSON file with new data.

**Parameters:**
- `file_path`: Path to the JSON file
- `new_data`: JSON object with new data

**Returns:** ERR_SUCCESS on success, error code on failure

#### `int merge_json_objects(cJSON *target, cJSON *source)`
Merges two JSON objects.

**Parameters:**
- `target`: Target JSON object
- `source`: Source JSON object

**Returns:** ERR_SUCCESS on success, error code on failure

#### `int create_history_entry(const char *file_path, cJSON *new_data)`
Creates a history entry for a JSON object.

**Parameters:**
- `file_path`: Path to the JSON file
- `new_data`: JSON object with new data

**Returns:** ERR_SUCCESS on success, error code on failure

## Utilities API (util.h)

### Functions

#### `int init_logger(const char *log_path)`
Initializes the logging system.

**Parameters:**
- `log_path`: Path to the log file

**Returns:** ERR_SUCCESS on success, error code on failure

#### `void log_message(int level, const char *format, ...)`
Logs a message with a specified level.

**Parameters:**
- `level`: Log level (LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG)
- `format`: Format string
- `...`: Variable arguments for format string

**Returns:** None

#### `char* read_file(const char *file_path)`
Reads the entire contents of a file into a string.

**Parameters:**
- `file_path`: Path to the file to read

**Returns:** String with file contents (caller must free) or NULL on failure

#### `int write_file(const char *file_path, const char *content)`
Writes a string to a file.

**Parameters:**
- `file_path`: Path to the file to write
- `content`: String to write to the file

**Returns:** ERR_SUCCESS on success, error code on failure

#### `void add_timestamp(cJSON *json_obj)`
Adds current timestamp to a JSON object.

**Parameters:**
- `json_obj`: JSON object to add timestamp to

**Returns:** None

#### `char* get_timestamp(char *buffer, size_t size)`
Gets a formatted timestamp string.

**Parameters:**
- `buffer`: Buffer to store the timestamp
- `size`: Size of the buffer

**Returns:** Pointer to the buffer or NULL on failure

## JSON Data Schema

### CPU Usage Object
```json
{
  "cpu_count": <integer>,
  "cpus": [
    {
      "name": <string>,
      "user": <integer>,
      "nice": <integer>,
      "system": <integer>,
      "idle": <integer>,
      "iowait": <integer>,
      "usage_percent": <float>
    },
    ...
  ]
}
```

### Memory Usage Object
```json
{
  "total_memory": <integer>,  // in bytes
  "free_memory": <integer>,   // in bytes
  "shared_memory": <integer>, // in bytes
  "buffer_memory": <integer>, // in bytes
  "usage_percent": <float>
}
```

### System Load Object
```json
{
  "load_1min": <float>,
  "load_5min": <float>,
  "load_15min": <float>
}
```

### Disk Usage Object
```json
{
  "filesystems": [
    {
      "mount_point": <string>,
      "size": <integer>,       // in bytes
      "used": <integer>,       // in bytes
      "available": <integer>,  // in bytes
      "usage_percent": <float>
    },
    ...
  ]
}
```

### Network Stats Object
```json
{
  "interfaces": [
    {
      "name": <string>,
      "rx_bytes": <integer>,
      "tx_bytes": <integer>,
      "rx_packets": <integer>,
      "tx_packets": <integer>,
      "rx_errors": <integer>,
      "tx_errors": <integer>
    },
    ...
  ]
}
```

### Uptime Object
```json
{
  "uptime_seconds": <integer>,
  "uptime_pretty": <string>
}
```

### Processes Object
```json
{
  "total_processes": <integer>,
  "running_processes": <integer>,
  "blocked_processes": <integer>,
  "sleeping_processes": <integer>
}
```

### Swap Usage Object
```json
{
  "total_swap": <integer>,   // in bytes
  "used_swap": <integer>,    // in bytes
  "free_swap": <integer>,    // in bytes
  "usage_percent": <float>
}
```

### History Entry Object
```json
{
  "timestamp": <string>,
  "cpu_usage": <CPU Usage Object>,
  "memory": <Memory Usage Object>,
  "load": <System Load Object>,
  "disk": <Disk Usage Object>,
  "network": <Network Stats Object>,
  "uptime": <Uptime Object>,
  "processes": <Processes Object>,
  "swap": <Swap Usage Object>
}
```

### Complete Data File Structure
```json
{
  "history": [
    <History Entry Object>,
    <History Entry Object>,
    ...
  ]
}
```

## Implementation Details

### Resource Collection

The application collects system information from various Linux system files:

- **CPU Usage**: `/proc/stat`
- **Memory Usage**: `/proc/meminfo`
- **System Load**: `getloadavg()` function or `/proc/loadavg`
- **Disk Usage**: `statvfs()` function
- **Network Statistics**: `/proc/net/dev`
- **Uptime**: `/proc/uptime`
- **Process Information**: `/proc` directory scanning
- **Swap Usage**: `/proc/meminfo`

### File Handling

The application reads and writes JSON files using the cJSON library, with additional utility functions for file I/O operations:

- `read_file()`: Uses standard C file I/O functions to read entire files
- `write_file()`: Uses standard C file I/O functions to write files

### Logging

The application implements a simple logging system that writes to a specified log file with timestamp and severity level information. Log messages are formatted as:
```
[YYYY-MM-DD HH:MM:SS] [LEVEL] Message
```

Where `LEVEL` is one of:
- `[ERROR]`: Critical errors
- `[WARNING]`: Non-critical issues
- `[INFO]`: Informational messages
- `[DEBUG]`: Debug information (only with verbose mode)