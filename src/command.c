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
    return cmd_srv;
}

u_int32_t command_service_register_handler(
        command_service_ptr service, 
        char *cmd, 
        size_t cmd_len, 
        command_type cmd_type, 
        handler_t handler) {
    
    LOG_DEBUG_F4("Add command handler %s %ld %d %d\n", cmd, cmd_len, cmd_type, service->command_count);
    command_info_ptr command;
    if (NULL == service || NULL == cmd || NULL == handler) {
        LOG_FATAL("FATAL command_service_register_handler params illegal\n");
        return -1;
    }
    if (FULL_MATCH != cmd_type && FIXED_PREFIX != cmd_type) {
        LOG_FATAL("FATAL command_service_register_handler param cmd type illegal\n");
        return -1;
    }
    if (FIXED_PREFIX == cmd_type && cmd_len <= 0 ) {
        LOG_FATAL("FATAL command_service_register_handler param cmd len illegal\n");
        return -1;
    }
    if (FULL_MATCH == cmd_type) {
        cmd_len = strlen(cmd);
    }
    command = (command_info_ptr)calloc(1, sizeof(command_info));
    if (NULL == command) {
        LOG_FATAL("FATAL command_service_register_handler calloc fail \n");
        return -1;
    }
    
    command->cmd = (char *)malloc((cmd_len + 1) * sizeof(char));
    if (NULL == command->cmd) {
        LOG_FATAL("FATAL command_service_register_handler malloc fail. \n");
        return -1;
    }
    if (FULL_MATCH == cmd_type) {
        strcpy(command->cmd, cmd);
    } else { 
        /* FIXED_PREFIX == cmd_type */
        strncpy(command->cmd, cmd, cmd_len);
        command->cmd[cmd_len] = '\0';
    } 
    command->cmd_len = cmd_len;
    command->type = cmd_type;
    command->handler = handler;
    if (NULL == service->command_list) {
        service->command_list = command;
        service->end_command = command;
    } else {
        service->end_command->next = command;
        service->end_command = command;
    }
    ++ service->command_count;
    LOG_DEBUG_F4("Added command %s %d %d %d\n", command->cmd, command->cmd_len, command->type, service->command_count);
    return 0;
} 

u_int32_t command_service_run(command_service_ptr service, char *cmd_s, int argc, char **argv) {
    command_info_ptr cmd;
    for (cmd = service->command_list; NULL != cmd; cmd = cmd->next) {
        if (FIXED_PREFIX == cmd->type && strncmp(cmd_s, cmd->cmd, cmd->cmd_len) == 0) {
            return cmd->handler(cmd_s, argc, argv);
        }
        if (FULL_MATCH == cmd->type && strcmp(cmd_s, cmd->cmd) == 0) {
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
