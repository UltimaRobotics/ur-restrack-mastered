/**
 * @file sysmon.h
 * @brief Header file for OpenWrt system resource monitoring application
 */

#ifndef SYSMON_H
#define SYSMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/sysinfo.h>

#include "cJSON.h"  /* Include cJSON header */

// Define default values
#define DEFAULT_CONFIG_PATH "/etc/sysmon_config.json"
#define DEFAULT_OUTPUT_PATH "/var/log/sysmon_data.json"
#define DEFAULT_LOG_PATH "/var/log/sysmon.log"
#define DEFAULT_COLLECTION_INTERVAL 5 // seconds

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

/**
 * @struct SysmonConfig
 * @brief Structure to hold configuration parameters for the system monitor
 */
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

// Function declarations
int init_logger(const char *log_path);
void log_message(int level, const char *format, ...);
int load_config(const char *config_path, SysmonConfig *config);
int save_config(const char *config_path, SysmonConfig *config);
void set_default_config(SysmonConfig *config);

// Resource collection functions declarations will be in resources.h

// Log levels
#define LOG_ERROR 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#endif /* SYSMON_H */
