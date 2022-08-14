#pragma once
#include "/home/petermiller/Desktop/Handmade-Kohi/defines.h"



#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#define LOG_WARN_ENABLED  1
#define LOG_INFO_ENABLED  1
#define LOG_DEBUG_ENABLED  1
#define LOG_TRACE_ENABLED  1

#if RELEASE == 1

#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

enum logLevel{

    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,

};



struct logger_system_state{

    b8 initialized;
    
};

static logger_system_state * state_ptr;

KINLINE b8 initializeLogging(u64 *memory_requirement, void * state){
    *memory_requirement  = sizeof(logger_system_state); 
    if(state == 0){

        return true;
    };

    state_ptr = (logger_system_state *)state;
    state_ptr->initialized = true;

return true;

};

KINLINE void shutdownLogging(void * state){

state_ptr  =0;

}
KINLINE void  platformConsoleWrite(const char * message, u8 colour){
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}
KINLINE void  platformConsoleWriteError(const char * message, u8 colour){

      const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

KINLINE KAPI void logOutput(logLevel level, const char * message, ...){
const char * levelStrings[6] = {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};
b8 isError  = level < 2;

char outMessage[32000];
memset(outMessage, 0, sizeof(outMessage));

__builtin_va_list argPtr;
va_start(argPtr, message);
vsnprintf(outMessage, 32000, message, argPtr);
va_end(argPtr);
char printMessage[32000];
sprintf(printMessage, "%s%s\n", levelStrings[level], outMessage);

// Platform-specific output.
    if (isError) {
        platformConsoleWriteError(printMessage, level);
    } else {
        platformConsoleWrite(printMessage, level);
    }
}

#ifndef KFATAl
#define KFATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#endif

#ifndef KERROR
#define KERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#endif

#if LOG_WARN_ENABLED == 1 
#define KWARN(message, ...) logOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
#define KWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1 
#define KINFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
#define KINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1 
#define KDEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
#define KDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1 
#define KTRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
#define KTRACE(message, ...)
#endif
