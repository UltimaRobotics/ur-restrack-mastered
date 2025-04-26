/**
 * @file util.c
 * @brief Utility functions for system monitoring application
 */

#include "util.h"
#include <stdarg.h>
#include <time.h>

// Global log file
static FILE *g_log_file = NULL;

/**
 * @brief Initialize the logging system
 * @param log_path Path to the log file
 * @return ERR_SUCCESS on success, error code on failure
 */
int init_logger(const char *log_path) {
    if (g_log_file != NULL) {
        fclose(g_log_file);
    }

    if (log_path == NULL || log_path[0] == '\0') {
        g_log_file = stderr;
        return ERR_SUCCESS;
    }

    g_log_file = fopen(log_path, "a");
    if (g_log_file == NULL) {
        fprintf(stderr, "Failed to open log file %s: %s\n", log_path, strerror(errno));
        g_log_file = stderr;
        return ERR_FILE_OPEN;
    }

    return ERR_SUCCESS;
}

/**
 * @brief Log a message with specified level
 * @param level Log level (LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG)
 * @param format Format string
 * @param ... Variable arguments for format string
 */
void log_message(int level, const char *format, ...) {
    if (g_log_file == NULL) {
        g_log_file = stderr;
    }

    // Get current time
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);

    // Get log level string
    const char *level_str;
    switch (level) {
        case LOG_ERROR:
            level_str = "ERROR";
            break;
        case LOG_WARNING:
            level_str = "WARNING";
            break;
        case LOG_INFO:
            level_str = "INFO";
            break;
        case LOG_DEBUG:
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    // Print timestamp and log level
    fprintf(g_log_file, "[%s] [%s] ", timestamp, level_str);

    // Print message
    va_list args;
    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);

    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

/**
 * @brief Read the entire contents of a file into a string
 * @param file_path Path to the file to read
 * @return String with file contents (caller must free) or NULL on failure
 */
char* read_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        log_message(LOG_ERROR, "Failed to open file %s: %s", file_path, strerror(errno));
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        log_message(LOG_ERROR, "Invalid file size for %s: %ld", file_path, file_size);
        fclose(file);
        return NULL;
    }

    // Allocate buffer for file contents
    char *buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        log_message(LOG_ERROR, "Memory allocation failed for reading %s", file_path);
        fclose(file);
        return NULL;
    }

    // Read file contents
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        log_message(LOG_ERROR, "Failed to read entire file %s: %s", file_path, strerror(errno));
        free(buffer);
        return NULL;
    }

    buffer[file_size] = '\0';
    return buffer;
}

/**
 * @brief Write a string to a file
 * @param file_path Path to the file to write
 * @param content String to write to the file
 * @return ERR_SUCCESS on success, error code on failure
 */
int write_file(const char *file_path, const char *content) {
    if (file_path == NULL || content == NULL) {
        return ERR_INVALID_PARAM;
    }

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        log_message(LOG_ERROR, "Failed to open file %s for writing: %s", file_path, strerror(errno));
        return ERR_FILE_OPEN;
    }

    // Write content to file
    size_t content_len = strlen(content);
    size_t bytes_written = fwrite(content, 1, content_len, file);
    
    fclose(file);

    if (bytes_written != content_len) {
        log_message(LOG_ERROR, "Failed to write entire content to %s: %s", file_path, strerror(errno));
        return ERR_FILE_WRITE;
    }

    return ERR_SUCCESS;
}

/**
 * @brief Add current timestamp to a JSON object
 * @param json_obj JSON object to add timestamp to
 */
void add_timestamp(cJSON *json_obj) {
    if (json_obj == NULL) {
        return;
    }

    // Get current time
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    
    // Format timestamp
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", lt);
    
    // Add timestamp to JSON object
    cJSON_AddStringToObject(json_obj, "timestamp", timestamp);
    
    // Also add Unix timestamp
    cJSON_AddNumberToObject(json_obj, "timestamp_unix", (double)t);
}

/**
 * @brief Get a formatted timestamp string
 * @param buffer Buffer to store the timestamp
 * @param size Size of the buffer
 * @return Pointer to the buffer or NULL on failure
 */
char* get_timestamp(char *buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return NULL;
    }

    // Get current time
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    
    // Format timestamp
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", lt);
    
    return buffer;
}
