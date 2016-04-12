#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "command.h"

command_service_ptr command_service_create() {
    command_service_ptr cmd_srv;
    cmd_srv = (command_service_ptr)calloc(1, sizeof(command_service));
    if (NULL == cmd_srv) {
        exit(-1);
    }
}

u_int32_t command_service_register_handler(command_service_ptr service,char *cmd, size_t cmd_len, handler_t handler) {
    command_info_ptr command;
    if (NULL == service || NULL == cmd || cmd_len <= 0 || NULL == handler) {
        return -1;
    }
    command = (command_info_ptr)calloc(1, sizeof(command_info));
    if (NULL == command) {
        exit(-1);
    }
    
    command->cmd = (char *)malloc((cmd_len + 1) * sizeof(char));
    if (NULL == command->cmd) {
        exit(-1);
    }
    strcpy(command->cmd, cmd);
    command->cmd_len = cmd_len;
    command->handler = handler;
    if (NULL == service->command_list) {
        service->command_list = command;
        service->end_command = command;
    } else {
        service->end_command->next = command;
        service->end_command = command;
    }
} 

u_int32_t command_service_run(command_service_ptr service, char *cmd_s, int argc, char **argv) {
    command_info_ptr cmd;
    for (cmd = service->command_list; NULL != cmd; cmd = cmd->next) {
        if (strncmp(cmd_s, cmd->cmd, cmd->cmd_len) == 0) {
            return cmd->handler(cmd_s, argc, argv);
        }
    }
    return -1;
}

void command_service_destory(command_service_ptr service) {
    command_info_ptr cmd, next_cmd;
    
    if (NULL == service) {
        return;
    }
    cmd = service->command_list;
    while (NULL != cmd) {
        next_cmd = cmd->next;
        free(cmd->cmd);
        free(cmd);
        cmd = next_cmd;
    }
}
