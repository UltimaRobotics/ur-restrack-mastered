/**
 * @file config.h
 * @brief Configuration handling functions for system monitoring application
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "sysmon.h"

/**
 * @brief Set default configuration values
 * @param config Pointer to configuration structure
 */
void set_default_config(SysmonConfig *config);

/**
 * @brief Load configuration from a JSON file
 * @param config_path Path to configuration file
 * @param config Pointer to configuration structure to populate
 * @return ERR_SUCCESS on success, error code on failure
 */
int load_config(const char *config_path, SysmonConfig *config);

/**
 * @brief Save configuration to a JSON file
 * @param config_path Path to configuration file
 * @param config Pointer to configuration structure
 * @return ERR_SUCCESS on success, error code on failure
 */
int save_config(const char *config_path, SysmonConfig *config);

/**
 * @brief Print configuration values
 * @param config Pointer to configuration structure
 */
void print_config(SysmonConfig *config);

#endif /* CONFIG_H */
