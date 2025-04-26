/**
 * @file resources.c
 * @brief System resource collection functions for monitoring application
 */

#include "resources.h"
#include "util.h"
#include <dirent.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <linux/if_link.h>

/**
 * @brief Collect all system resources based on configuration
 * @param config Pointer to configuration structure
 * @return cJSON object with collected resources or NULL on failure
 */
cJSON* collect_all_resources(SysmonConfig *config) {
    if (config == NULL) {
        log_message(LOG_ERROR, "Invalid configuration");
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        log_message(LOG_ERROR, "Failed to create JSON object");
        return NULL;
    }

    // Collect resources based on configuration
    if (config->collect_cpu) {
        cJSON *cpu_data = collect_cpu_usage();
        if (cpu_data != NULL) {
            cJSON_AddItemToObject(root, "cpu_usage", cpu_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect CPU usage");
        }
    }

    if (config->collect_memory) {
        cJSON *memory_data = collect_memory_usage();
        if (memory_data != NULL) {
            cJSON_AddItemToObject(root, "memory_usage", memory_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect memory usage");
        }
    }

    if (config->collect_load) {
        cJSON *load_data = collect_system_load();
        if (load_data != NULL) {
            cJSON_AddItemToObject(root, "system_load", load_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect system load");
        }
    }

    if (config->collect_disk) {
        cJSON *disk_data = collect_disk_usage();
        if (disk_data != NULL) {
            cJSON_AddItemToObject(root, "disk_usage", disk_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect disk usage");
        }
    }

    if (config->collect_network) {
        cJSON *network_data = collect_network_stats();
        if (network_data != NULL) {
            cJSON_AddItemToObject(root, "network_stats", network_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect network statistics");
        }
    }

    if (config->collect_uptime) {
        cJSON *uptime_data = collect_system_uptime();
        if (uptime_data != NULL) {
            cJSON_AddItemToObject(root, "system_uptime", uptime_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect system uptime");
        }
    }

    if (config->collect_processes) {
        cJSON *process_data = collect_process_info();
        if (process_data != NULL) {
            cJSON_AddItemToObject(root, "process_info", process_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect process information");
        }
    }

    if (config->collect_swap) {
        cJSON *swap_data = collect_swap_usage();
        if (swap_data != NULL) {
            cJSON_AddItemToObject(root, "swap_usage", swap_data);
        } else {
            log_message(LOG_WARNING, "Failed to collect swap usage");
        }
    }

    return root;
}

/**
 * @brief Collect CPU usage information
 * @return cJSON object with CPU usage data or NULL on failure
 */
cJSON* collect_cpu_usage(void) {
    FILE *stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL) {
        log_message(LOG_ERROR, "Failed to open /proc/stat: %s", strerror(errno));
        return NULL;
    }

    cJSON *cpu_data = cJSON_CreateObject();
    if (cpu_data == NULL) {
        fclose(stat_file);
        return NULL;
    }

    // Add number of CPU cores
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus > 0) {
        cJSON_AddNumberToObject(cpu_data, "cpu_count", num_cpus);
    }

    // Parse CPU statistics
    char line[256];
    cJSON *cpus_array = cJSON_CreateArray();
    if (cpus_array == NULL) {
        cJSON_Delete(cpu_data);
        fclose(stat_file);
        return NULL;
    }

    while (fgets(line, sizeof(line), stat_file)) {
        char cpu_name[16];
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        
        // Read CPU line (cpu0, cpu1, etc.)
        if (sscanf(line, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", 
                   cpu_name, &user, &nice, &system, &idle, &iowait, 
                   &irq, &softirq, &steal, &guest, &guest_nice) >= 5) {
            
            if (strncmp(cpu_name, "cpu", 3) == 0) {
                // Skip the aggregate CPU line (just "cpu")
                if (strlen(cpu_name) == 3) {
                    continue;
                }
                
                cJSON *cpu_obj = cJSON_CreateObject();
                if (cpu_obj == NULL) {
                    continue;
                }
                
                cJSON_AddStringToObject(cpu_obj, "name", cpu_name);
                cJSON_AddNumberToObject(cpu_obj, "user", user);
                cJSON_AddNumberToObject(cpu_obj, "nice", nice);
                cJSON_AddNumberToObject(cpu_obj, "system", system);
                cJSON_AddNumberToObject(cpu_obj, "idle", idle);
                cJSON_AddNumberToObject(cpu_obj, "iowait", iowait);
                
                // Calculate total time
                unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal;
                if (total > 0) {
                    double usage_pct = 100.0 * (total - idle) / total;
                    cJSON_AddNumberToObject(cpu_obj, "usage_percent", usage_pct);
                }
                
                cJSON_AddItemToArray(cpus_array, cpu_obj);
            }
        }
    }
    
    fclose(stat_file);
    
    // Add the CPUs array to the main object
    cJSON_AddItemToObject(cpu_data, "cpus", cpus_array);
    
    return cpu_data;
}

/**
 * @brief Collect memory usage information
 * @return cJSON object with memory usage data or NULL on failure
 */
cJSON* collect_memory_usage(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        log_message(LOG_ERROR, "Failed to get system info: %s", strerror(errno));
        return NULL;
    }

    cJSON *memory_data = cJSON_CreateObject();
    if (memory_data == NULL) {
        return NULL;
    }

    // Convert to MB for readability
    unsigned long total_mb = info.totalram * info.mem_unit / (1024 * 1024);
    unsigned long free_mb = info.freeram * info.mem_unit / (1024 * 1024);
    unsigned long used_mb = total_mb - free_mb;
    
    cJSON_AddNumberToObject(memory_data, "total_mb", total_mb);
    cJSON_AddNumberToObject(memory_data, "used_mb", used_mb);
    cJSON_AddNumberToObject(memory_data, "free_mb", free_mb);
    
    if (total_mb > 0) {
        double usage_pct = 100.0 * used_mb / total_mb;
        cJSON_AddNumberToObject(memory_data, "usage_percent", usage_pct);
    }

    return memory_data;
}

/**
 * @brief Collect system load information
 * @return cJSON object with system load data or NULL on failure
 */
cJSON* collect_system_load(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        log_message(LOG_ERROR, "Failed to get system info: %s", strerror(errno));
        return NULL;
    }

    cJSON *load_data = cJSON_CreateObject();
    if (load_data == NULL) {
        return NULL;
    }

    // Get load averages (1, 5, and 15 minutes)
    // Convert from fixed point (1/65536) to float
    double load1 = info.loads[0] / 65536.0;
    double load5 = info.loads[1] / 65536.0;
    double load15 = info.loads[2] / 65536.0;
    
    cJSON_AddNumberToObject(load_data, "load1", load1);
    cJSON_AddNumberToObject(load_data, "load5", load5);
    cJSON_AddNumberToObject(load_data, "load15", load15);

    // Add number of running processes
    cJSON_AddNumberToObject(load_data, "running_processes", info.procs);

    return load_data;
}

/**
 * @brief Collect disk usage and IO information
 * @return cJSON object with disk usage data or NULL on failure
 */
cJSON* collect_disk_usage(void) {
    cJSON *disk_data = cJSON_CreateObject();
    if (disk_data == NULL) {
        return NULL;
    }

    // Get filesystem statistics
    cJSON *filesystems = cJSON_CreateArray();
    if (filesystems == NULL) {
        cJSON_Delete(disk_data);
        return NULL;
    }

    // Get root filesystem statistics as a minimum
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        cJSON *fs = cJSON_CreateObject();
        if (fs != NULL) {
            unsigned long total_mb = (stat.f_blocks * stat.f_frsize) / (1024 * 1024);
            unsigned long free_mb = (stat.f_bfree * stat.f_frsize) / (1024 * 1024);
            unsigned long used_mb = total_mb - free_mb;
            
            cJSON_AddStringToObject(fs, "mount_point", "/");
            cJSON_AddNumberToObject(fs, "total_mb", total_mb);
            cJSON_AddNumberToObject(fs, "used_mb", used_mb);
            cJSON_AddNumberToObject(fs, "free_mb", free_mb);
            
            if (total_mb > 0) {
                double usage_pct = 100.0 * used_mb / total_mb;
                cJSON_AddNumberToObject(fs, "usage_percent", usage_pct);
            }
            
            cJSON_AddItemToArray(filesystems, fs);
        }
    }

    // Try to read disk IO statistics
    FILE *diskstats = fopen("/proc/diskstats", "r");
    if (diskstats != NULL) {
        cJSON *io_stats = cJSON_CreateArray();
        if (io_stats != NULL) {
            char line[256];
            
            while (fgets(line, sizeof(line), diskstats) != NULL) {
                int major, minor;
                char dev_name[32];
                unsigned long reads, reads_merged, sectors_read, read_time;
                unsigned long writes, writes_merged, sectors_written, write_time;
                unsigned long io_pending, io_time, weighted_io_time;
                
                // Parse diskstats line
                if (sscanf(line, "%d %d %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                          &major, &minor, dev_name,
                          &reads, &reads_merged, &sectors_read, &read_time,
                          &writes, &writes_merged, &sectors_written, &write_time,
                          &io_pending, &io_time, &weighted_io_time) == 14) {
                    
                    // Skip loop, ram and dm devices
                    if (strncmp(dev_name, "loop", 4) == 0 ||
                        strncmp(dev_name, "ram", 3) == 0 ||
                        strncmp(dev_name, "dm-", 3) == 0) {
                        continue;
                    }
                    
                    cJSON *dev = cJSON_CreateObject();
                    if (dev != NULL) {
                        cJSON_AddStringToObject(dev, "device", dev_name);
                        cJSON_AddNumberToObject(dev, "reads", reads);
                        cJSON_AddNumberToObject(dev, "writes", writes);
                        cJSON_AddNumberToObject(dev, "read_sectors", sectors_read);
                        cJSON_AddNumberToObject(dev, "written_sectors", sectors_written);
                        
                        // 512 bytes is the traditional sector size
                        unsigned long read_kb = sectors_read / 2;
                        unsigned long written_kb = sectors_written / 2;
                        
                        cJSON_AddNumberToObject(dev, "read_kb", read_kb);
                        cJSON_AddNumberToObject(dev, "written_kb", written_kb);
                        
                        cJSON_AddItemToArray(io_stats, dev);
                    }
                }
            }
            
            cJSON_AddItemToObject(disk_data, "io_stats", io_stats);
        }
        
        fclose(diskstats);
    }

    cJSON_AddItemToObject(disk_data, "filesystems", filesystems);
    return disk_data;
}

/**
 * @brief Collect network statistics
 * @return cJSON object with network statistics or NULL on failure
 */
cJSON* collect_network_stats(void) {
    cJSON *network_data = cJSON_CreateObject();
    if (network_data == NULL) {
        return NULL;
    }

    // Get network interface statistics
    cJSON *interfaces = cJSON_CreateArray();
    if (interfaces == NULL) {
        cJSON_Delete(network_data);
        return NULL;
    }

    // Read from /proc/net/dev which has network interface statistics
    FILE *netdev = fopen("/proc/net/dev", "r");
    if (netdev == NULL) {
        log_message(LOG_ERROR, "Failed to open /proc/net/dev: %s", strerror(errno));
        cJSON_Delete(network_data);
        return NULL;
    }

    char line[256];
    // Skip the first two header lines
    fgets(line, sizeof(line), netdev);
    fgets(line, sizeof(line), netdev);

    while (fgets(line, sizeof(line), netdev)) {
        char *colon = strchr(line, ':');
        if (colon == NULL) {
            continue;
        }

        *colon = '\0';
        char *iface_name = line;
        // Trim leading whitespace
        while (isspace(*iface_name)) {
            iface_name++;
        }

        // Parse interface statistics
        char *stats = colon + 1;
        unsigned long rx_bytes, rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_compressed, rx_multicast;
        unsigned long tx_bytes, tx_packets, tx_errs, tx_drop, tx_fifo, tx_colls, tx_carrier, tx_compressed;

        if (sscanf(stats, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                  &rx_bytes, &rx_packets, &rx_errs, &rx_drop, &rx_fifo, &rx_frame, &rx_compressed, &rx_multicast,
                  &tx_bytes, &tx_packets, &tx_errs, &tx_drop, &tx_fifo, &tx_colls, &tx_carrier, &tx_compressed) == 16) {
            
            cJSON *iface = cJSON_CreateObject();
            if (iface != NULL) {
                cJSON_AddStringToObject(iface, "interface", iface_name);
                
                // Add receive statistics
                cJSON *rx = cJSON_CreateObject();
                if (rx != NULL) {
                    cJSON_AddNumberToObject(rx, "bytes", rx_bytes);
                    cJSON_AddNumberToObject(rx, "packets", rx_packets);
                    cJSON_AddNumberToObject(rx, "errors", rx_errs);
                    cJSON_AddNumberToObject(rx, "dropped", rx_drop);
                    cJSON_AddItemToObject(iface, "receive", rx);
                }
                
                // Add transmit statistics
                cJSON *tx = cJSON_CreateObject();
                if (tx != NULL) {
                    cJSON_AddNumberToObject(tx, "bytes", tx_bytes);
                    cJSON_AddNumberToObject(tx, "packets", tx_packets);
                    cJSON_AddNumberToObject(tx, "errors", tx_errs);
                    cJSON_AddNumberToObject(tx, "dropped", tx_drop);
                    cJSON_AddItemToObject(iface, "transmit", tx);
                }
                
                cJSON_AddItemToArray(interfaces, iface);
            }
        }
    }

    fclose(netdev);
    cJSON_AddItemToObject(network_data, "interfaces", interfaces);
    return network_data;
}

/**
 * @brief Collect system uptime information
 * @return cJSON object with uptime data or NULL on failure
 */
cJSON* collect_system_uptime(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        log_message(LOG_ERROR, "Failed to get system info: %s", strerror(errno));
        return NULL;
    }

    cJSON *uptime_data = cJSON_CreateObject();
    if (uptime_data == NULL) {
        return NULL;
    }

    // Calculate days, hours, minutes, seconds
    unsigned long uptime_seconds = info.uptime;
    unsigned int days = uptime_seconds / (60 * 60 * 24);
    unsigned int hours = (uptime_seconds % (60 * 60 * 24)) / (60 * 60);
    unsigned int minutes = (uptime_seconds % (60 * 60)) / 60;
    unsigned int seconds = uptime_seconds % 60;

    cJSON_AddNumberToObject(uptime_data, "total_seconds", uptime_seconds);
    cJSON_AddNumberToObject(uptime_data, "days", days);
    cJSON_AddNumberToObject(uptime_data, "hours", hours);
    cJSON_AddNumberToObject(uptime_data, "minutes", minutes);
    cJSON_AddNumberToObject(uptime_data, "seconds", seconds);

    // Format human-readable uptime
    char uptime_str[64];
    if (days > 0) {
        snprintf(uptime_str, sizeof(uptime_str), "%u days %u hours %u minutes", days, hours, minutes);
    } else if (hours > 0) {
        snprintf(uptime_str, sizeof(uptime_str), "%u hours %u minutes", hours, minutes);
    } else {
        snprintf(uptime_str, sizeof(uptime_str), "%u minutes %u seconds", minutes, seconds);
    }
    
    cJSON_AddStringToObject(uptime_data, "uptime", uptime_str);

    return uptime_data;
}

/**
 * @brief Collect process information
 * @return cJSON object with process information or NULL on failure
 */
cJSON* collect_process_info(void) {
    cJSON *process_data = cJSON_CreateObject();
    if (process_data == NULL) {
        return NULL;
    }

    // Get number of processes
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        log_message(LOG_ERROR, "Failed to open /proc directory: %s", strerror(errno));
        cJSON_Delete(process_data);
        return NULL;
    }

    int process_count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(proc_dir)) != NULL) {
        // Count only numeric directories (PIDs)
        /* Skip . and .. entries, and only check directories with numeric names (PID dirs) */
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0' && pid > 0) {
                process_count++;
            }
        }
    }
    
    closedir(proc_dir);
    cJSON_AddNumberToObject(process_data, "count", process_count);

    // Try to get number of running processes
    FILE *stat_file = fopen("/proc/stat", "r");
    if (stat_file != NULL) {
        char line[256];
        int procs_running = 0;
        int procs_blocked = 0;
        
        while (fgets(line, sizeof(line), stat_file)) {
            if (strncmp(line, "procs_running", 13) == 0) {
                sscanf(line, "procs_running %d", &procs_running);
            } else if (strncmp(line, "procs_blocked", 13) == 0) {
                sscanf(line, "procs_blocked %d", &procs_blocked);
            }
        }
        
        fclose(stat_file);
        
        cJSON_AddNumberToObject(process_data, "running", procs_running);
        cJSON_AddNumberToObject(process_data, "blocked", procs_blocked);
    }

    return process_data;
}

/**
 * @brief Collect swap usage information
 * @return cJSON object with swap usage data or NULL on failure
 */
cJSON* collect_swap_usage(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        log_message(LOG_ERROR, "Failed to get system info: %s", strerror(errno));
        return NULL;
    }

    cJSON *swap_data = cJSON_CreateObject();
    if (swap_data == NULL) {
        return NULL;
    }

    // Convert to MB for readability
    unsigned long total_mb = info.totalswap * info.mem_unit / (1024 * 1024);
    unsigned long free_mb = info.freeswap * info.mem_unit / (1024 * 1024);
    unsigned long used_mb = total_mb - free_mb;
    
    cJSON_AddNumberToObject(swap_data, "total_mb", total_mb);
    cJSON_AddNumberToObject(swap_data, "used_mb", used_mb);
    cJSON_AddNumberToObject(swap_data, "free_mb", free_mb);
    
    if (total_mb > 0) {
        double usage_pct = 100.0 * used_mb / total_mb;
        cJSON_AddNumberToObject(swap_data, "usage_percent", usage_pct);
    }

    return swap_data;
}
