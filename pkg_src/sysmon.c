/**
 * @file sysmon.c
 * @brief Main file for OpenWrt system resource monitoring application
 */

#include "sysmon.h"
#include "config.h"
#include "resources.h"
#include "json_handler.h"
#include "util.h"
#include <getopt.h>  /* For getopt, optarg, optopt */

// Global configuration
SysmonConfig g_config;

/**
 * @brief Print usage information
 */
void print_usage(char *progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  -c <config_file>   Specify configuration file (default: %s)\n", DEFAULT_CONFIG_PATH);
    printf("  -o <output_file>   Specify output file (default: from config or %s)\n", DEFAULT_OUTPUT_PATH);
    printf("  -l <log_file>      Specify log file (default: from config or %s)\n", DEFAULT_LOG_PATH);
    printf("  -i <interval>      Specify collection interval in seconds (default: from config or %d)\n", DEFAULT_COLLECTION_INTERVAL);
    printf("  -v                 Enable verbose output\n");
    printf("  -x                 Run once and exit\n");
    printf("  -h                 Display this help message\n");
}

/**
 * @brief Main function
 */
int main(int argc, char *argv[]) {
    int c;
    char config_path[256] = DEFAULT_CONFIG_PATH;
    char output_path[256] = {0};
    char log_path[256] = {0};
    int interval = 0;
    int verbose = 0;
    int run_once = 0;

    // Parse command line arguments
    while ((c = getopt(argc, argv, "c:o:l:i:vhx")) != -1) {
        switch (c) {
            case 'c':
                strncpy(config_path, optarg, sizeof(config_path) - 1);
                break;
            case 'o':
                strncpy(output_path, optarg, sizeof(output_path) - 1);
                break;
            case 'l':
                strncpy(log_path, optarg, sizeof(log_path) - 1);
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'x':
                run_once = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                if (optopt == 'c' || optopt == 'o' || optopt == 'l' || optopt == 'i')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }
    }

    // Set default config
    set_default_config(&g_config);

    // Load config from file
    int config_status = load_config(config_path, &g_config);
    if (config_status != ERR_SUCCESS && config_status != ERR_FILE_OPEN) {
        fprintf(stderr, "Error loading configuration: %d\n", config_status);
        return 1;
    }

    // Override config with command line options if provided
    if (output_path[0] != '\0') {
        strncpy(g_config.output_path, output_path, sizeof(g_config.output_path) - 1);
        g_config.output_path[sizeof(g_config.output_path) - 1] = '\0'; // Ensure null termination
    }
    if (log_path[0] != '\0') {
        strncpy(g_config.log_path, log_path, sizeof(g_config.log_path) - 1);
        g_config.log_path[sizeof(g_config.log_path) - 1] = '\0'; // Ensure null termination
    }
    if (interval > 0) {
        g_config.collection_interval = interval;
    }
    if (verbose) {
        g_config.verbose = verbose;
    }

    // Initialize logger
    if (init_logger(g_config.log_path) != ERR_SUCCESS) {
        fprintf(stderr, "Error initializing logger\n");
        return 1;
    }

    log_message(LOG_INFO, "System monitoring started with interval: %d seconds", g_config.collection_interval);
    log_message(LOG_INFO, "Output file: %s", g_config.output_path);

    // Main monitoring loop
    while (1) {
        // Collect system resource data
        cJSON *resource_data = collect_all_resources(&g_config);
        if (resource_data == NULL) {
            log_message(LOG_ERROR, "Failed to collect system resources");
            sleep(g_config.collection_interval);
            continue;
        }

        // Add timestamp to data
        add_timestamp(resource_data);

        // Update or create the JSON file
        int result = update_json_file(g_config.output_path, resource_data);
        if (result != ERR_SUCCESS) {
            log_message(LOG_ERROR, "Failed to update JSON file: %d", result);
        } else {
            log_message(LOG_INFO, "Successfully updated system resource data");
        }

        // Free JSON resources
        cJSON_Delete(resource_data);

        // Exit if running once, otherwise sleep
        if (run_once) {
            break;
        }
        
        sleep(g_config.collection_interval);
    }

    log_message(LOG_INFO, "System monitoring stopped");
    return 0;
}
