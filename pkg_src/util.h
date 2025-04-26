/**
 * @file util.h
 * @brief Utility functions for system monitoring application
 */

#ifndef UTIL_H
#define UTIL_H

#include "cJSON.h"
#include "sysmon.h"
#include <stdarg.h>

/**
 * @brief Initialize the logging system
 * @param log_path Path to the log file
 * @return ERR_SUCCESS on success, error code on failure
 */
int init_logger(const char *log_path);

/**
 * @brief Log a message with specified level
 * @param level Log level (LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG)
 * @param format Format string
 * @param ... Variable arguments for format string
 */
void log_message(int level, const char *format, ...);

/**
 * @brief Read the entire contents of a file into a string
 * @param file_path Path to the file to read
 * @return String with file contents (caller must free) or NULL on failure
 */
char* read_file(const char *file_path);

/**
 * @brief Write a string to a file
 * @param file_path Path to the file to write
 * @param content String to write to the file
 * @return ERR_SUCCESS on success, error code on failure
 */
int write_file(const char *file_path, const char *content);

/**
 * @brief Add current timestamp to a JSON object
 * @param json_obj JSON object to add timestamp to
 */
void add_timestamp(cJSON *json_obj);

/**
 * @brief Get a formatted timestamp string
 * @param buffer Buffer to store the timestamp
 * @param size Size of the buffer
 * @return Pointer to the buffer or NULL on failure
 */
char* get_timestamp(char *buffer, size_t size);

#endif /* UTIL_H */
