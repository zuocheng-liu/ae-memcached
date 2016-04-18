#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include "settings.h"
#include "global.h"

/*
 * Macro functions
 */
/* Logger functions */
#define LOG_PRINT(format) LOG_PRINT_F0(format)
#define LOG_PRINT_F0(format)                                \
    fprintf(stderr, format)
#define LOG_PRINT_F1(format,arg0)                           \
    fprintf(stderr, format, arg0)
#define LOG_PRINT_F2(format,arg0, arg1)                     \
    fprintf(stderr, format, arg0, arg1)
#define LOG_PRINT_F3(format,arg0, arg1, arg2)               \
    fprintf(stderr, format, arg0, arg1, arg2)
#define LOG_PRINT_F4(format,arg0, arg1, arg2, arg3)         \
    fprintf(stderr, format, arg0, arg1, arg2, arg3)

#define LOG_DEBUG(format) LOG_DEBUG_F0(format) 
#define LOG_DEBUG_F0(format)                                \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F0(format);                           \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F1(format, arg0)                          \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F1(format, arg0);                     \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F2(format, arg0, arg1)                    \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F2(format, arg0, arg1);               \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F3(format, arg0, arg1, arg2)              \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F3(format, arg0, arg1, arg2);         \
        }                                                   \
    } while (0)
#define LOG_DEBUG_F4(format, arg0, arg1, arg2, arg3)        \
    do {                                                    \
        if (settings.verbose > 1) {                         \
            LOG_PRINT_F4(format, arg0, arg1, arg2, arg3);   \
        }                                                   \
    } while (0)


#define LOG_INFO(format) LOG_INFO_F0(format) 
#define LOG_INFO_F0(format)                                 \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F0(format);                           \
        }                                                   \
    } while (0)
#define LOG_INFO_F1(format, arg0)                           \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F1(format, arg0);                     \
        }                                                   \
    } while (0)
#define LOG_INFO_F2(format, arg0, arg1)                     \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F2(format, arg0, arg1);               \
        }                                                   \
    } while (0)
#define LOG_INFO_F3(format, arg0, arg1, arg2)               \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F3(format, arg0, arg1, arg2);         \
        }                                                   \
    } while (0)
#define LOG_INFO_F4(format, arg0, arg1, arg2, arg3)         \
    do {                                                    \
        if (settings.verbose > 0) {                         \
            LOG_PRINT_F4(format, arg0 ,arg1, arg2, arg3);   \
        }                                                   \
    } while (0)

#define LOG_WARN(format) LOG_WARN_F0(format) 
#define LOG_WARN_F0(format)                                 \
    do {                                                    \
        LOG_PRINT_F0(format);                               \
    } while (0)
#define LOG_WARN_F1(format, arg0)                           \
    do {                                                    \
        LOG_PRINT_F1(format, arg0);                         \
    } while (0)

#define LOG_ERROR(format) LOG_ERROR_F0(format) 
#define LOG_ERROR_F0(format)                                \
    do {                                                    \
        LOG_PRINT_F0(format);                               \
    } while (0)
#define LOG_ERROR_F1(format, arg0)                          \
    do {                                                    \
        LOG_PRINT_F1(format, arg0);                         \
    } while (0)


#define LOG_FATAL(format) LOG_FATAL_F0(format) 
#define LOG_FATAL_F0(format)                                \
    do {                                                    \
        LOG_PRINT_F0(format);                                  \
        exit(-1);                                           \
    } while (0)
#define LOG_FATAL_F1(format, arg0)                          \
    do {                                                    \
        LOG_PRINT_F1(format, arg0);                            \
        exit(-1);                                           \
    } while (0)

#endif /* end definition of LOGGER_H */
