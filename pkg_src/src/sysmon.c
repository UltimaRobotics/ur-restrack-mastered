/**
 * @file sysmon.c
 * @brief Main file for OpenWrt system resource monitoring application
 */
#include "util.h"
#include <getopt.h>  /* For getopt, optarg, optopt */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>  

#include <ur-restrack.h>  /* Include ur-restrack header */
#include <pthread.h>  /* For pthread_create, pthread_join */
#include "sysmon.h"
#include "config.h"
#include "resources.h"
#include "json_handler.h"

void on_message(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
    MqttThreadContext* context_temp = (MqttThreadContext*)userdata;
    Config_mqtt* config = &context_temp->config_base;
    pthread_mutex_lock(&context_temp->mutex);

    if (message->payloadlen) {
        for (int i = 0; i < context_temp->config_additional.json_added_subs.topics_num; i++) {
            if (strcmp(message->topic, context_temp->config_additional.json_added_subs.topics[i]) == 0) {
                if (strcmp(message->topic, RESTRACK_ACTION_TOPIC) == 0) {
                    cJSON* cmd_json = cJSON_Parse((char*)message->payload);
                    if (cmd_json) {
                        restrack_cmd_t cmd;
                        memset(&cmd, 0, sizeof(cmd));
                        
                        cJSON* new_config_json = cJSON_GetObjectItemCaseSensitive(cmd_json, "new_config");
                        if (new_config_json && cJSON_IsObject(new_config_json)) {
                            cJSON* output_path = cJSON_GetObjectItemCaseSensitive(new_config_json, "output_path");
                            if (output_path && cJSON_IsString(output_path)) {
                                strncpy(cmd.new_config.output_path, output_path->valuestring, sizeof(cmd.new_config.output_path) - 1);
                            }
                            cJSON* log_path = cJSON_GetObjectItemCaseSensitive(new_config_json, "log_path");
                            if (log_path && cJSON_IsString(log_path)) {
                                strncpy(cmd.new_config.log_path, log_path->valuestring, sizeof(cmd.new_config.log_path) - 1);
                            }
                            cJSON* interval = cJSON_GetObjectItemCaseSensitive(new_config_json, "collection_interval");
                            if (interval && cJSON_IsNumber(interval)) {
                                cmd.new_config.collection_interval = interval->valueint;
                            }
                            cJSON* verbose = cJSON_GetObjectItemCaseSensitive(new_config_json, "verbose");
                            if (verbose && cJSON_IsBool(verbose)) {
                                cmd.new_config.verbose = cJSON_IsTrue(verbose);
                            }
                            cJSON* collect_cpu = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_cpu");
                            if (collect_cpu && cJSON_IsBool(collect_cpu)) {
                                cmd.new_config.collect_cpu = cJSON_IsTrue(collect_cpu);
                            }
                            cJSON* collect_memory = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_memory");
                            if (collect_memory && cJSON_IsBool(collect_memory)) {
                                cmd.new_config.collect_memory = cJSON_IsTrue(collect_memory);
                            }
                            cJSON* collect_load = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_load");
                            if (collect_load && cJSON_IsBool(collect_load)) {
                                cmd.new_config.collect_load = cJSON_IsTrue(collect_load);
                            }
                            cJSON* collect_disk = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_disk");
                            if (collect_disk && cJSON_IsBool(collect_disk)) {
                                cmd.new_config.collect_disk = cJSON_IsTrue(collect_disk);
                            }
                            cJSON* collect_network = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_network");
                            if (collect_network && cJSON_IsBool(collect_network)) {
                                cmd.new_config.collect_network = cJSON_IsTrue(collect_network);
                            }
                            cJSON* collect_uptime = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_uptime");
                            if (collect_uptime && cJSON_IsBool(collect_uptime)) {
                                cmd.new_config.collect_uptime = cJSON_IsTrue(collect_uptime);
                            }
                            cJSON* collect_processes = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_processes");
                            if (collect_processes && cJSON_IsBool(collect_processes)) {
                                cmd.new_config.collect_processes = cJSON_IsTrue(collect_processes);
                            }
                            cJSON* collect_swap = cJSON_GetObjectItemCaseSensitive(new_config_json, "collect_swap");
                            if (collect_swap && cJSON_IsBool(collect_swap)) {
                                cmd.new_config.collect_swap = cJSON_IsTrue(collect_swap);
                            }
                        }
                        cJSON* action_json = cJSON_GetObjectItemCaseSensitive(cmd_json, "action");
                        if (action_json && cJSON_IsString(action_json)) {
                            cmd.action = string_to_action(action_json->valuestring);
                        } else {
                            cmd.action = UPDATE;
                        }
                        handle_restrack_action(&cmd,g_args);
                    }
                    cJSON_Delete(cmd_json);
                } 
                break;
            }
        }
    }
    pthread_mutex_unlock(&context_temp->mutex);
}

void on_connect(struct mosquitto* mosq, void* obj, int rc) {
    if (rc == 0) {
        fprintf(stderr, "[MQTT] Connected successfully\n");
        for (int i = 0; i < context->config_additional.json_added_subs.topics_num; i++) {
            mosquitto_subscribe(mosq, NULL, context->config_additional.json_added_subs.topics[i], 0);
            #ifdef _DEBUG
                printf(" [MQTT] context->config_additional.json_added_subs.topics[%d]:%s\n",i,context->config_additional.json_added_subs.topics[i]);
            #endif
        }
    } else {
        fprintf(stderr, "[MQTT] Connection failed: %s\n", mosquitto_strerror(rc));
    }
}

void* mqtt_reconnect_func(void *arg){
    MqttThreadContext* context = (MqttThreadContext*)arg;
    if (context == NULL) {
        fprintf(stderr, "Context is NULL\n");
        return NULL;
    }
    while (1) {
        int rc = mosquitto_loop(context->mosq, 100, 1);
        if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_NO_CONN) {
            fprintf(stderr, "[MQTT] Connection error: %s\n", mosquitto_strerror(rc));
            sleep(1);
            mosquitto_reconnect(context->mosq);
        }
    }
    return NULL;
}


const char *thread_state_to_string(thread_state_t state) {
    switch (state) {
        case THREAD_CREATED:
            return "CREATED";
        case THREAD_RUNNING:
            return "RUNNING";
        case THREAD_PAUSED:
            return "PAUSED";
        case THREAD_STOPPED:
            return "STOPPED";
        case THREAD_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void list_threads() {
    unsigned int count = thread_get_count(&manager);
    printf("\n=== Threads (%u) ===\n", count);
    
    unsigned int *ids = (unsigned int *)malloc(count * sizeof(unsigned int));
    int num_ids = thread_get_all_ids(&manager, ids, count);
    
    if (num_ids == 0) {
        printf("No threads running.\n");
    } else {
        printf("ID\tState\tAlive\n");
        printf("--\t-----\t-----\n");
        
        for (int i = 0; i < num_ids; i++) {
            thread_state_t state;
            thread_get_state(&manager, ids[i], &state);
            bool alive = thread_is_alive(&manager, ids[i]);
            
            printf("%u\t%s\t%s\n", 
                   ids[i], 
                   thread_state_to_string(state), 
                   alive ? "Yes" : "No");
        }
    }
    free(ids);
}

int main(int argc, char *argv[]) {
    #ifdef _DEBUG
        printf("[DEBUG] Entering main function\n");
    #endif
    
    if (argc != 4) {
    #ifdef _DEBUG
        printf("[DEBUG] Incorrect number of arguments (%d), returning EXIT_FAILURE\n", argc);
    #endif
        return EXIT_FAILURE;
    }
    #ifdef _DEBUG
        printf("[DEBUG] Arguments received: %s %s %s\n", argv[1], argv[2], argv[3]);
    #endif
    SysmonArgs args = {DEFAULT_CONFIG_PATH, {0}, {0}, 0, 0, 0, argv[1]};
    g_args = malloc(sizeof(SysmonArgs));
    memset(g_args, 0, sizeof(SysmonArgs));
    // g_args->json_file = (char*)malloc(256*sizeof(char));
    g_args = &args;

    #ifdef _DEBUG
        printf("[DEBUG] SysmonArgs global args initialised , current json file %s\n", g_args->json_file);
    #endif
    
    if (thread_manager_init(&manager, 10) != 0) {
        #ifdef _DEBUG
            printf("[DEBUG] Failed to initialize thread manager\n");
        #endif
        fprintf(stderr, "Failed to initialize thread manager\n");
        return 1;
    }
    #ifdef _DEBUG
        printf("[DEBUG] Thread manager initialized successfully\n");
    #endif
    printf("Thread manager initialized\n");
    context = malloc(sizeof(MqttThreadContext));
    memset(context, 0, sizeof(MqttThreadContext));
    pthread_mutex_init(&context->mutex, NULL);
    
    #ifdef _DEBUG
        printf("[DEBUG] Allocated and initialized MQTT thread context\n");
    #endif
    
    context->config_paths.base_config_path = strdup(argv[2]);
    context->config_paths.custom_config_path = strdup(argv[3]);
    
    #ifdef _DEBUG
        printf("[DEBUG] Set config paths: base=%s, custom=%s\n", 
               context->config_paths.base_config_path, 
               context->config_paths.custom_config_path);
    #endif
    
    context->config_base = parse_base_config(argv[2]);
    context->config_additional = parse_custom_topics(argv[3]);
    
    #ifdef _DEBUG
        printf("[DEBUG] Parsed base and custom configs\n");
    #endif
    
    context->mqtt_monitor.last_activity = time(NULL);
    atomic_init(&context->mqtt_monitor.running, false);
    atomic_init(&context->mqtt_monitor.healthy, false);
    atomic_init(&context->health_monitor.running, false);
    
    #ifdef _DEBUG
        printf("[DEBUG] Initialized monitor variables\n");
    #endif
    
    launch_thread(mqtt_thread_func, context);
    #ifdef _DEBUG
        printf("[DEBUG] MQTT thread launched\n");
    #endif
        
    launch_heartbeat_thread(function_heartbeat);
    launch_target_thread(restrack_runner_func,&args);
    

    #ifdef _DEBUG
        printf("[DEBUG] Entering main loop\n");
    #endif
    
    while (running) {
        list_threads();
        sleep(1);
    }
    
    #ifdef _DEBUG
        printf("[DEBUG] Cleanup started\n");
    #endif
    atomic_store(&context->mqtt_monitor.running, false);
    atomic_store(&context->health_monitor.running, false);
    sleep(1);
    free_base_config(&context->config_base);
    free_custom_topics(&context->config_additional);
    free(context->config_paths.base_config_path);
    free(context->config_paths.custom_config_path);
    pthread_mutex_destroy(&context->mutex);
    thread_manager_destroy(&manager);
    free(context);
            
    #ifdef _DEBUG
        printf("[DEBUG] Cleanup complete, exiting\n");
    #endif
        return 0;
    }