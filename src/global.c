#include "global.h"

struct aeEventLoop *g_el;
command_service_ptr g_cmd_srv;

struct settings settings;
struct stats stats;

volatile rel_time_t current_time;
