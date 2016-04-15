#include "global.h"

int l_socket = 0;
int u_socket = -1;

struct aeEventLoop *g_el;
command_service_ptr g_cmd_srv;

struct settings settings;
struct stats stats;

volatile rel_time_t current_time;
