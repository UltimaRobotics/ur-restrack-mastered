/**
 * @file resources.h
 * @brief System resource collection functions for monitoring application
 */

#ifndef RESOURCES_H
#define RESOURCES_H

#include "cJSON.h"
#include "sysmon.h"

/**
 * @brief Collect all system resources based on configuration
 * @param config Pointer to configuration structure
 * @return cJSON object with collected resources or NULL on failure
 */
cJSON* collect_all_resources(SysmonConfig *config);

/**
 * @brief Collect CPU usage information
 * @return cJSON object with CPU usage data or NULL on failure
 */
cJSON* collect_cpu_usage(void);

/**
 * @brief Collect memory usage information
 * @return cJSON object with memory usage data or NULL on failure
 */
cJSON* collect_memory_usage(void);

/**
 * @brief Collect system load information
 * @return cJSON object with system load data or NULL on failure
 */
cJSON* collect_system_load(void);

/**
 * @brief Collect disk usage and IO information
 * @return cJSON object with disk usage data or NULL on failure
 */
cJSON* collect_disk_usage(void);

/**
 * @brief Collect network statistics
 * @return cJSON object with network statistics or NULL on failure
 */
cJSON* collect_network_stats(void);

/**
 * @brief Collect system uptime information
 * @return cJSON object with uptime data or NULL on failure
 */
cJSON* collect_system_uptime(void);

/**
 * @brief Collect process information
 * @return cJSON object with process information or NULL on failure
 */
cJSON* collect_process_info(void);

/**
 * @brief Collect swap usage information
 * @return cJSON object with swap usage data or NULL on failure
 */
cJSON* collect_swap_usage(void);

#endif /* RESOURCES_H */
