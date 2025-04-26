/**
 * @file config.c
 * @brief Configuration handling functions for system monitoring application
 */

#include "config.h"
#include "util.h"
#include "json_handler.h"

/**
 * @brief Set default configuration values
 * @param config Pointer to configuration structure
 */
void set_default_config(SysmonConfig *config) {
    if (config == NULL) {
        return;
    }

    strncpy(config->output_path, DEFAULT_OUTPUT_PATH, sizeof(config->output_path) - 1);
    strncpy(config->log_path, DEFAULT_LOG_PATH, sizeof(config->log_path) - 1);
    config->collection_interval = DEFAULT_COLLECTION_INTERVAL;
    config->verbose = 0;
    
    // Enable all resource collections by default
    config->collect_cpu = 1;
    config->collect_memory = 1;
    config->collect_load = 1;
    config->collect_disk = 1;
    config->collect_network = 1;
    config->collect_uptime = 1;
    config->collect_processes = 1;
    config->collect_swap = 1;
}

/**
 * @brief Load configuration from a JSON file
 * @param config_path Path to configuration file
 * @param config Pointer to configuration structure to populate
 * @return ERR_SUCCESS on success, error code on failure
 */
int load_config(const char *config_path, SysmonConfig *config) {
    if (config_path == NULL || config == NULL) {
        return ERR_INVALID_PARAM;
    }

    char *json_str = read_file(config_path);
    if (json_str == NULL) {
        log_message(LOG_WARNING, "Could not read config file, using defaults");
        return ERR_FILE_OPEN;
    }

    cJSON *root = cJSON_Parse(json_str);
    free(json_str);

    if (root == NULL) {
        log_message(LOG_ERROR, "Error parsing config JSON");
        return ERR_JSON_PARSE;
    }

    // Parse configuration values
    cJSON *output_path = cJSON_GetObjectItem(root, "output_path");
    if (output_path != NULL && cJSON_IsString(output_path)) {
        strncpy(config->output_path, output_path->valuestring, sizeof(config->output_path) - 1);
    }

    cJSON *log_path = cJSON_GetObjectItem(root, "log_path");
    if (log_path != NULL && cJSON_IsString(log_path)) {
        strncpy(config->log_path, log_path->valuestring, sizeof(config->log_path) - 1);
    }

    cJSON *collection_interval = cJSON_GetObjectItem(root, "collection_interval");
    if (collection_interval != NULL && cJSON_IsNumber(collection_interval)) {
        config->collection_interval = collection_interval->valueint;
    }

    cJSON *verbose = cJSON_GetObjectItem(root, "verbose");
    if (verbose != NULL && cJSON_IsBool(verbose)) {
        config->verbose = cJSON_IsTrue(verbose);
    }

    // Collection flags
    cJSON *collect_cpu = cJSON_GetObjectItem(root, "collect_cpu");
    if (collect_cpu != NULL && cJSON_IsBool(collect_cpu)) {
        config->collect_cpu = cJSON_IsTrue(collect_cpu);
    }

    cJSON *collect_memory = cJSON_GetObjectItem(root, "collect_memory");
    if (collect_memory != NULL && cJSON_IsBool(collect_memory)) {
        config->collect_memory = cJSON_IsTrue(collect_memory);
    }

    cJSON *collect_load = cJSON_GetObjectItem(root, "collect_load");
    if (collect_load != NULL && cJSON_IsBool(collect_load)) {
        config->collect_load = cJSON_IsTrue(collect_load);
    }

    cJSON *collect_disk = cJSON_GetObjectItem(root, "collect_disk");
    if (collect_disk != NULL && cJSON_IsBool(collect_disk)) {
        config->collect_disk = cJSON_IsTrue(collect_disk);
    }

    cJSON *collect_network = cJSON_GetObjectItem(root, "collect_network");
    if (collect_network != NULL && cJSON_IsBool(collect_network)) {
        config->collect_network = cJSON_IsTrue(collect_network);
    }

    cJSON *collect_uptime = cJSON_GetObjectItem(root, "collect_uptime");
    if (collect_uptime != NULL && cJSON_IsBool(collect_uptime)) {
        config->collect_uptime = cJSON_IsTrue(collect_uptime);
    }

    cJSON *collect_processes = cJSON_GetObjectItem(root, "collect_processes");
    if (collect_processes != NULL && cJSON_IsBool(collect_processes)) {
        config->collect_processes = cJSON_IsTrue(collect_processes);
    }

    cJSON *collect_swap = cJSON_GetObjectItem(root, "collect_swap");
    if (collect_swap != NULL && cJSON_IsBool(collect_swap)) {
        config->collect_swap = cJSON_IsTrue(collect_swap);
    }

    cJSON_Delete(root);
    return ERR_SUCCESS;
}

/**
 * @brief Save configuration to a JSON file
 * @param config_path Path to configuration file
 * @param config Pointer to configuration structure
 * @return ERR_SUCCESS on success, error code on failure
 */
int save_config(const char *config_path, SysmonConfig *config) {
    if (config_path == NULL || config == NULL) {
        return ERR_INVALID_PARAM;
    }

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        return ERR_JSON_CREATE;
    }

    // Add configuration values to JSON
    cJSON_AddStringToObject(root, "output_path", config->output_path);
    cJSON_AddStringToObject(root, "log_path", config->log_path);
    cJSON_AddNumberToObject(root, "collection_interval", config->collection_interval);
    cJSON_AddBoolToObject(root, "verbose", config->verbose);

    // Add collection flags
    cJSON_AddBoolToObject(root, "collect_cpu", config->collect_cpu);
    cJSON_AddBoolToObject(root, "collect_memory", config->collect_memory);
    cJSON_AddBoolToObject(root, "collect_load", config->collect_load);
    cJSON_AddBoolToObject(root, "collect_disk", config->collect_disk);
    cJSON_AddBoolToObject(root, "collect_network", config->collect_network);
    cJSON_AddBoolToObject(root, "collect_uptime", config->collect_uptime);
    cJSON_AddBoolToObject(root, "collect_processes", config->collect_processes);
    cJSON_AddBoolToObject(root, "collect_swap", config->collect_swap);

    // Convert JSON to string and save to file
    char *json_str = cJSON_Print(root);
    if (json_str == NULL) {
        cJSON_Delete(root);
        return ERR_JSON_CREATE;
    }

    int result = write_file(config_path, json_str);
    free(json_str);
    cJSON_Delete(root);

    return result;
}

/**
 * @brief Print configuration values
 * @param config Pointer to configuration structure
 */
void print_config(SysmonConfig *config) {
    if (config == NULL) {
        return;
    }

    printf("Configuration:\n");
    printf("  Output path: %s\n", config->output_path);
    printf("  Log path: %s\n", config->log_path);
    printf("  Collection interval: %d seconds\n", config->collection_interval);
    printf("  Verbose: %s\n", config->verbose ? "Yes" : "No");
    printf("  Collections enabled:\n");
    printf("    CPU: %s\n", config->collect_cpu ? "Yes" : "No");
    printf("    Memory: %s\n", config->collect_memory ? "Yes" : "No");
    printf("    Load: %s\n", config->collect_load ? "Yes" : "No");
    printf("    Disk: %s\n", config->collect_disk ? "Yes" : "No");
    printf("    Network: %s\n", config->collect_network ? "Yes" : "No");
    printf("    Uptime: %s\n", config->collect_uptime ? "Yes" : "No");
    printf("    Processes: %s\n", config->collect_processes ? "Yes" : "No");
    printf("    Swap: %s\n", config->collect_swap ? "Yes" : "No");
}
