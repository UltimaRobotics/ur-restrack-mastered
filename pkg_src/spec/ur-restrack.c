#include "sysmon.h"
#include "config.h"
#include "resources.h"
#include "json_handler.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>

#include "ur-restrack.h"

#include <signal.h> 

SysmonConfig g_config;
SysmonArgs* g_args = NULL;
unsigned int RUNNER_TRACKER ;
thread_manager_t manager;
volatile sig_atomic_t running = 1;

void* function_heartbeat(void *args){
    mqttthreadder_context_t* temp_ = (mqttthreadder_context_t*)args;
    MqttThreadContext *context = temp_->context;
    if (context == NULL) {
        fprintf(stderr, "Context is NULL\n");
        return NULL;
    }
    unsigned int thread_id = 0;
    for (unsigned int i = 0; i < thread_get_count(&manager); i++) {
        unsigned int *ids = (unsigned int *)malloc(thread_get_count(&manager) * sizeof(unsigned int));
        int count = thread_get_all_ids(&manager, ids, thread_get_count(&manager));
        for (int j = 0; j < count; j++) {
            thread_info_t info;
            if (thread_get_info(&manager, ids[j], &info) == 0) {
                if (info.arg == args) {
                    thread_id = ids[j];
                    break;
                }
            }
        }
        free(ids);
        if (thread_id != 0) {
            break;
        }
    }
    printf("Heartbeat thread ID: %u\n", thread_id);
    while (1) {
        publish_to_custom_topic(RESTRACK_HEARTBEAT_TOPIC, RESTRACK_HEARTBEAT_MESSAGE);
        sleep(1);
    }

}

const char* action_to_string(ur_restrack_action action) {
    switch(action) {
        case UPDATE: return "UPDATE";
        case RESTART: return "RESTART";
        case SHUTDOWN: return "SHUTDOWN";
        default: return "UNKNOWN";
    }
}

ur_restrack_action string_to_action(const char* str) {
    if (strcmp(str, "UPDATE") == 0) return UPDATE;
    if (strcmp(str, "RESTART") == 0) return RESTART;
    if (strcmp(str, "SHUTDOWN") == 0) return SHUTDOWN;
    return UPDATE;
}



#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

void debug_print_thread_state(thread_state_t state, int use_colors) {
    const char *state_strings[] = {
        [THREAD_CREATED] = "CREATED",
        [THREAD_RUNNING] = "RUNNING",
        [THREAD_PAUSED]  = "PAUSED",
        [THREAD_STOPPED] = "STOPPED",
        [THREAD_ERROR]   = "ERROR"
    };
    
    const char *colors[] = {
        [THREAD_CREATED] = COLOR_BLUE,
        [THREAD_RUNNING] = COLOR_GREEN,
        [THREAD_PAUSED]  = COLOR_YELLOW,
        [THREAD_STOPPED] = COLOR_RESET,
        [THREAD_ERROR]   = COLOR_RED
    };

    if (state >= THREAD_CREATED && state <= THREAD_ERROR) {
        if (use_colors) {
            printf("[DEBUG] Thread state: %s%s%s\n", 
                   colors[state], state_strings[state], COLOR_RESET);
        } else {
            printf("[DEBUG] Thread state: %s\n", state_strings[state]);
        }
    } else {
        printf("[DEBUG] Invalid thread state: %d\n", state);
    }
}

char* sysmon_config_to_json(const SysmonConfig* config) {
    if (!config) return NULL;

    cJSON* root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON_AddStringToObject(root, "output_path", config->output_path);
    cJSON_AddStringToObject(root, "log_path", config->log_path);
    
    cJSON_AddNumberToObject(root, "collection_interval", config->collection_interval);
    
    cJSON_AddBoolToObject(root, "verbose", config->verbose ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_cpu", config->collect_cpu ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_memory", config->collect_memory ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_load", config->collect_load ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_disk", config->collect_disk ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_network", config->collect_network ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_uptime", config->collect_uptime ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_processes", config->collect_processes ? 1 : 0);
    cJSON_AddBoolToObject(root, "collect_swap", config->collect_swap ? 1 : 0);

    char* json_string = cJSON_Print(root);
    cJSON_Delete(root);
    
    return json_string;
}

void init_global_args(SysmonArgs* g_args) {
    if (g_args == NULL) {
        g_args = (SysmonArgs*)calloc(1, sizeof(SysmonArgs));
        if (!g_args) {
            fprintf(stderr, "Failed to allocate global args\n");
            exit(EXIT_FAILURE);
        }
        g_args->json_file = NULL;
    }
}

void cleanup_global_args(SysmonArgs* g_args) {
    if (g_args) {
        free(g_args->json_file);
        
        free(g_args);
        g_args = NULL;  
    }
}

char *create_config_file(const char *config,size_t len) {
    if (config == NULL) {
        return NULL;
    }
    char path[255];
    snprintf(path, sizeof(path), "/tmp/restrack-config-XXXXXX");
    int fd = mkstemp(path);
    if (fd == -1) { return NULL;  }
    const char *p = config;
    while (len > 0) {
        ssize_t n = write(fd, p, len);
        if (n < 0) {
            if (errno == EINTR) { continue; }
            close(fd);unlink(path);
            return NULL;
        } else if (n == 0) {
            close(fd);unlink(path);
            return NULL;
        }
        len -= n;
        p += n;
    }
    if (close(fd) == -1) {
        unlink(path);return NULL;
    }
    char *result = strdup(path);
    if (!result) {
        unlink(path);return NULL;
    }
    return result;
}

void* restrack_runner_func(void *arg) {

    SysmonArgs* args = (SysmonArgs*)arg;
    unsigned int thread_id = 0;
    for (unsigned int i = 0; i < thread_get_count(&manager); i++) {
        unsigned int *ids = (unsigned int *)malloc(thread_get_count(&manager) * sizeof(unsigned int));
        int count = thread_get_all_ids(&manager, ids, thread_get_count(&manager));
        for (int j = 0; j < count; j++) {
            thread_info_t info;
            if (thread_get_info(&manager, ids[j], &info) == 0) {
                if (info.arg == args) {
                    thread_id = ids[j];
                    break;
                }
            }
        }
        free(ids);
        if (thread_id != 0) {
            break;
        }
    }
    char *config_path = args->config_path;
    char *output_path = args->output_path;
    char *log_path = args->log_path;
    int interval = args->interval;
    int verbose = args->verbose;
    int run_once = args->run_once;

    char *json_file = args->json_file;
    set_default_config(&g_config);
    #ifdef _DEBUG
        printf("[DEBUG] Loading data from restrack config file :%s\n",args->json_file);
    #endif
    int config_status = load_config(args->json_file, &g_config);
    if (config_status != ERR_SUCCESS && config_status != ERR_FILE_OPEN) {
        fprintf(stderr, "Error loading configuration: %d\n", config_status);
        return 1;
    }

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

    if (init_logger(g_config.log_path) != ERR_SUCCESS) {
        fprintf(stderr, "Error initializing logger\n");
        return 1;
    }

    log_message(LOG_INFO, "System monitoring started with interval: %d seconds", g_config.collection_interval);
    log_message(LOG_INFO, "Output file: %s", g_config.output_path);


    while (1) {
        if (thread_should_exit(&manager, thread_id)) {
            return NULL;
        }
        cJSON *resource_data = collect_all_resources(&g_config);
        if (resource_data == NULL) {
            log_message(LOG_ERROR, "Failed to collect system resources");
            sleep(g_config.collection_interval);
            continue;
        }
        add_timestamp(resource_data);
        int result = update_json_file(g_config.output_path, resource_data);
        if (result != ERR_SUCCESS) {
            log_message(LOG_ERROR, "Failed to update JSON file: %d", result);
        } else {
            log_message(LOG_INFO, "Successfully updated system resource data");
        }
        publish_to_custom_topic(RESTRACK_STATUS_TOPIC,cJSON_Print(resource_data));
        cJSON_Delete(resource_data);

        if (run_once) {
            break;
        }
        sleep(g_config.collection_interval);

    }

    log_message(LOG_INFO, "System monitoring stopped");
}

void launch_heartbeat_thread(void* func){
    int *thread_num = (int *)malloc(sizeof(int));
    *thread_num = thread_get_count(&manager) + 1;
    unsigned int new_id  = *thread_num;
    
    mqttthreadder_context_t *context_temp_ = (mqttthreadder_context_t *)malloc(sizeof(mqttthreadder_context_t));
    context_temp_->thread_id = new_id;
    context_temp_->context= context;

    if (thread_create(&manager, func, context_temp_, &context_temp_->thread_id) > 0) {
        #ifdef _DEBUG
                printf("[DEBUG] Heartbeat thread created with ID: %u\n", context_temp_->thread_id);
        #endif
    } else {
        #ifdef _DEBUG
                printf("[DEBUG] Failed to create heartbeat thread\n");
        #endif
        fprintf(stderr, "Failed to create heartbeat thread\n");
        free(thread_num);
    }
}

void * launch_thread(void* func ,void* args){
    int *thread_num = (int *)malloc(sizeof(int));
    *thread_num = thread_get_count(&manager) + 1;
    unsigned int new_id  = *thread_num;

    if (thread_create(&manager, func, args, &new_id) > 0) {
        #ifdef _DEBUG
                printf("[DEBUG] Generic Launch thread created with ID: %u\n", new_id);
        #endif
    } else {
        #ifdef _DEBUG
                printf("[DEBUG] Failed to create thread\n");
        #endif
        free(thread_num);
    }
}


void * launch_target_thread(void* func ,void* args){
    int *thread_num = (int *)malloc(sizeof(int));
    *thread_num = thread_get_count(&manager) + 1;
    unsigned int new_id  = *thread_num;

    if (thread_create(&manager, func, args, &new_id) > 0) {
        #ifdef _DEBUG
                printf("[DEBUG] Target Launch thread created with ID: %u\n", new_id);
        #endif

    } else {
        #ifdef _DEBUG
                printf("[DEBUG] Failed to create thread\n");
        #endif
        free(thread_num);
    }
}

void handle_restrack_action(restrack_cmd_t* temp_cmd , SysmonArgs* g_args) {
    if (temp_cmd == NULL) {
        fprintf(stderr, "Received NULL command\n");
        return;
    }

    unsigned int count = thread_get_count(&manager);
    unsigned int *ids = (unsigned int *)malloc(count * sizeof(unsigned int));
    int num_ids = thread_get_all_ids(&manager, ids, count);
    unsigned int runner_latest = ids[num_ids-1];
    free(ids);

    if (temp_cmd->action == UPDATE) {
        char* config_json = sysmon_config_to_json(&temp_cmd->new_config);
        if (!config_json) {
            fprintf(stderr, "Failed to convert config to JSON\n");
            return;
        }
        printf ("[DEBUG] JSON config : %s\n",config_json);
        char* config_path = create_config_file(config_json,strlen(config_json));
        
        if (!config_path) {
            fprintf(stderr, "Failed to create config file\n");
            return;
        }
        #ifdef _DEBUG
            printf("[DEBUG] Created new config file :%s\n",config_path);
        #endif
        SysmonArgs* temp_g_args = (SysmonArgs *)malloc(sizeof(SysmonArgs));
        init_global_args(temp_g_args);
        temp_g_args->json_file = (char *)malloc(256*sizeof(char));
        memset(temp_g_args->json_file,0,256*sizeof(char));
        temp_g_args->json_file = strdup(config_path); 

        if (!temp_g_args->json_file) {
            fprintf(stderr, "Failed to duplicate config path\n");
        }        
        #ifdef _DEBUG
            printf("[DEBUG] runner_latest affected value : %u\n", runner_latest );
        #endif
        thread_stop(&manager, runner_latest);
        #ifdef _DEBUG
            printf("[DEBUG] Stopped Target Thread\n"); 
        #endif
        #ifdef _DEBUG
        printf("[DEBUG] New config loader %s\n",temp_g_args->json_file);
        #endif
        thread_create(&manager,restrack_runner_func,temp_g_args,&runner_latest);
        #ifdef _DEBUG
            printf("[DEBUG] Restarted thread tracker with new config\n");
        #endif
        g_args = temp_g_args; 
        free(config_json);  
        
    } else if (temp_cmd->action == SHUTDOWN) {
        #ifdef _DEBUG
            printf("[DEBUG] Shutting down thread tracker\n");
        #endif
        
        if (thread_stop(&manager, runner_latest) == 0) {
            #ifdef _DEBUG
                printf("[DEBUG] Stopped Target Thread\n"); 
            #endif
        } else {
            #ifdef _DEBUG
                printf("[DEBUG] Failed to stop Target Thread error id : %d\n",thread_stop(&manager, runner_latest));
            #endif
        }
        

    } else {
        #ifdef _DEBUG
            printf("[DEBUG] Restarting thread tracker with new config\n");
        #endif
        
        thread_stop(&manager, runner_latest);
        thread_restart(&manager, runner_latest, g_args);	

    }
}
