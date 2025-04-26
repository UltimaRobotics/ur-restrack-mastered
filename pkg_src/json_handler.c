/**
 * @file json_handler.c
 * @brief JSON handling functions for system monitoring application
 */

#include "json_handler.h"
#include "util.h"

/**
 * @brief Update a JSON file with new data
 * @param file_path Path to the JSON file
 * @param new_data JSON object with new data
 * @return ERR_SUCCESS on success, error code on failure
 */
int update_json_file(const char *file_path, cJSON *new_data) {
    if (file_path == NULL || new_data == NULL) {
        return ERR_INVALID_PARAM;
    }

    cJSON *root = NULL;
    char *json_str = NULL;
    int result = ERR_SUCCESS;

    // Try to read existing file
    json_str = read_file(file_path);
    if (json_str != NULL) {
        root = cJSON_Parse(json_str);
        free(json_str);
        
        if (root == NULL) {
            log_message(LOG_WARNING, "Failed to parse existing JSON file %s, creating new one", file_path);
        }
    }

    // If no existing data or failed to parse, create new object
    if (root == NULL) {
        root = cJSON_CreateObject();
        if (root == NULL) {
            log_message(LOG_ERROR, "Failed to create new JSON object");
            return ERR_JSON_CREATE;
        }
        
        // Add current data to the root
        if (merge_json_objects(root, new_data) != ERR_SUCCESS) {
            log_message(LOG_ERROR, "Failed to merge data into new root object");
            cJSON_Delete(root);
            return ERR_JSON_CREATE;
        }
        
        // Add history array
        cJSON *history = cJSON_CreateArray();
        if (history == NULL) {
            log_message(LOG_ERROR, "Failed to create history array");
            cJSON_Delete(root);
            return ERR_JSON_CREATE;
        }
        
        // Add current data as first history entry
        cJSON *first_entry = cJSON_Duplicate(new_data, 1);
        if (first_entry == NULL) {
            log_message(LOG_ERROR, "Failed to duplicate data for history");
            cJSON_Delete(history);
            cJSON_Delete(root);
            return ERR_JSON_CREATE;
        }
        
        cJSON_AddItemToArray(history, first_entry);
        cJSON_AddItemToObject(root, "history", history);
    } else {
        // Update current data
        if (merge_json_objects(root, new_data) != ERR_SUCCESS) {
            log_message(LOG_ERROR, "Failed to merge data into existing root object");
            cJSON_Delete(root);
            return ERR_JSON_CREATE;
        }
        
        // Add to history
        cJSON *history = cJSON_GetObjectItem(root, "history");
        if (history == NULL || !cJSON_IsArray(history)) {
            log_message(LOG_WARNING, "No valid history array found, creating new one");
            
            history = cJSON_CreateArray();
            if (history == NULL) {
                log_message(LOG_ERROR, "Failed to create history array");
                cJSON_Delete(root);
                return ERR_JSON_CREATE;
            }
            
            cJSON_AddItemToObject(root, "history", history);
        }
        
        // Add new entry to history (limited to 100 entries)
        if (cJSON_GetArraySize(history) >= 100) {
            // Remove oldest entry
            cJSON_DeleteItemFromArray(history, 0);
        }
        
        cJSON *new_entry = cJSON_Duplicate(new_data, 1);
        if (new_entry == NULL) {
            log_message(LOG_ERROR, "Failed to duplicate data for history");
            cJSON_Delete(root);
            return ERR_JSON_CREATE;
        }
        
        cJSON_AddItemToArray(history, new_entry);
    }

    // Convert to string and save
    json_str = cJSON_Print(root);
    if (json_str == NULL) {
        log_message(LOG_ERROR, "Failed to convert JSON to string");
        cJSON_Delete(root);
        return ERR_JSON_CREATE;
    }
    
    result = write_file(file_path, json_str);
    free(json_str);
    cJSON_Delete(root);
    
    return result;
}

/**
 * @brief Merge two JSON objects
 * @param target Target JSON object
 * @param source Source JSON object
 * @return ERR_SUCCESS on success, error code on failure
 */
int merge_json_objects(cJSON *target, cJSON *source) {
    if (target == NULL || source == NULL) {
        return ERR_INVALID_PARAM;
    }

    // Iterate through all items in source
    cJSON *item = NULL;
    cJSON *existing_item = NULL;
    const char *key = NULL;
    
    cJSON_ArrayForEach(item, source) {
        key = item->string;
        
        // Skip if this is the history item
        if (key != NULL && strcmp(key, "history") == 0) {
            continue;
        }
        
        // Check if key already exists in target
        existing_item = cJSON_GetObjectItem(target, key);
        if (existing_item != NULL) {
            // Replace existing item
            cJSON_DeleteItemFromObject(target, key);
        }
        
        // Add new item (duplicated to avoid double free)
        cJSON *new_item = cJSON_Duplicate(item, 1);
        if (new_item == NULL) {
            log_message(LOG_ERROR, "Failed to duplicate JSON item");
            return ERR_JSON_CREATE;
        }
        
        cJSON_AddItemToObject(target, key, new_item);
    }
    
    return ERR_SUCCESS;
}

/**
 * @brief Create a history entry for a JSON object
 * @param file_path Path to the JSON file
 * @param new_data JSON object with new data
 * @return ERR_SUCCESS on success, error code on failure
 */
int create_history_entry(const char *file_path, cJSON *new_data) {
    if (file_path == NULL || new_data == NULL) {
        return ERR_INVALID_PARAM;
    }

    cJSON *root = NULL;
    char *json_str = NULL;
    int result = ERR_SUCCESS;

    // Try to read existing file
    json_str = read_file(file_path);
    if (json_str == NULL) {
        log_message(LOG_WARNING, "Failed to read file %s, creating new one", file_path);
        root = cJSON_CreateObject();
    } else {
        root = cJSON_Parse(json_str);
        free(json_str);
        
        if (root == NULL) {
            log_message(LOG_WARNING, "Failed to parse existing JSON file %s, creating new one", file_path);
            root = cJSON_CreateObject();
        }
    }

    if (root == NULL) {
        log_message(LOG_ERROR, "Failed to create JSON object");
        return ERR_JSON_CREATE;
    }

    // Get or create history array
    cJSON *history = cJSON_GetObjectItem(root, "history");
    if (history == NULL || !cJSON_IsArray(history)) {
        history = cJSON_CreateArray();
        if (history == NULL) {
            log_message(LOG_ERROR, "Failed to create history array");
            cJSON_Delete(root);
            return ERR_JSON_CREATE;
        }
        
        cJSON_AddItemToObject(root, "history", history);
    }

    // Add new entry to history (limited to 100 entries)
    if (cJSON_GetArraySize(history) >= 100) {
        // Remove oldest entry
        cJSON_DeleteItemFromArray(history, 0);
    }
    
    cJSON *new_entry = cJSON_Duplicate(new_data, 1);
    if (new_entry == NULL) {
        log_message(LOG_ERROR, "Failed to duplicate data for history");
        cJSON_Delete(root);
        return ERR_JSON_CREATE;
    }
    
    cJSON_AddItemToArray(history, new_entry);

    // Convert to string and save
    json_str = cJSON_Print(root);
    if (json_str == NULL) {
        log_message(LOG_ERROR, "Failed to convert JSON to string");
        cJSON_Delete(root);
        return ERR_JSON_CREATE;
    }
    
    result = write_file(file_path, json_str);
    free(json_str);
    cJSON_Delete(root);
    
    return result;
}
