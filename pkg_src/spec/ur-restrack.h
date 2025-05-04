#include "sysmon.h"
#include "config.h"
#include "resources.h"
#include "json_handler.h"
#include "ur-rpc-template.h"
#include <signal.h>

#include "thread_manager.h"
#include <time.h>  

extern SysmonConfig g_config;

extern unsigned int RUNNER_TRACKER ;


typedef struct {
    char config_path[256];
    char output_path[256];
    char log_path[256];
    int interval;
    int verbose;
    int run_once;
    char *json_file;
} SysmonArgs; 

extern SysmonArgs* g_args ;


typedef enum {
    UPDATE,
    RESTART,
    SHUTDOWN
} ur_restrack_action;

typedef struct {
    SysmonConfig new_config;
    ur_restrack_action action;
} restrack_cmd_t;

#define RESTRACK_ACTION_TOPIC "ur-restrack-actions"
#define RESTRACK_RESULT_TOPIC "ur-restrack-results"
#define RESTRACK_STATUS_TOPIC  "ur-restrack-status"
#define RESTRACK_HEARTBEAT_TOPIC "ur-restrack-heartbeat"
#define RESTRACK_HEARTBEAT_MESSAGE "restrack_heartbeat"

extern thread_manager_t manager;
extern volatile sig_atomic_t running ;

#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 199309L)
extern int nanosleep(const struct timespec *req, struct timespec *rem);
#endif

static void signal_handler(int signo);

typedef struct {
    unsigned int thread_id;
    MqttThreadContext *context;
} mqttthreadder_context_t;

void * restrack_runner_func(void *arg);
void *function_heartbeat(void *args);

char *create_config_file(const char *config,size_t size);
const char* action_to_string(ur_restrack_action action);


void cleanup_global_args(SysmonArgs* g_args);
void cleanup_global_args(SysmonArgs* g_args);
char *create_config_file(const char *config,size_t len);


char* sysmon_config_to_json(const SysmonConfig* config) ;
void* restrack_runner_func(void *arg);

void handle_restrack_action(restrack_cmd_t* temp_cmd , SysmonArgs* g_args);

void launch_heartbeat_thread(void* func);
void * launch_thread(void* func ,void* args);
void * launch_target_thread(void* func ,void* args);