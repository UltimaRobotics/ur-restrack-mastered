/**
 * @file json_handler.h
 * @brief JSON handling functions for system monitoring application
 */

#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include "cJSON.h"
#include "sysmon.h"

/**
 * @brief Update a JSON file with new data
 * @param file_path Path to the JSON file
 * @param new_data JSON object with new data
 * @return ERR_SUCCESS on success, error code on failure
 */
int update_json_file(const char *file_path, cJSON *new_data);

/**
 * @brief Merge two JSON objects
 * @param target Target JSON object
 * @param source Source JSON object
 * @return ERR_SUCCESS on success, error code on failure
 */
int merge_json_objects(cJSON *target, cJSON *source);

/**
 * @brief Create a history entry for a JSON object
 * @param file_path Path to the JSON file
 * @param new_data JSON object with new data
 * @return ERR_SUCCESS on success, error code on failure
 */
int create_history_entry(const char *file_path, cJSON *new_data);

#endif /* JSON_HANDLER_H */
