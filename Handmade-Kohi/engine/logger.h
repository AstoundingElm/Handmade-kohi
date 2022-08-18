#pragma once

#include "../defines.h"
#include "../shaders/filesystem.h"

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

    file_handle log_file_handle;
    
};

global_variable logger_system_state * logger_state_ptr;

KINLINE u64 stringLength(const char * string);

KINLINE void* kzeroMemory(void* block, u64 size);
KINLINE i32 string_format(char* dest, const char* format, ...);
 template<typename T> KINLINE i32 string_format_v(char* dest, const char* format, T* va_listp);

KINLINE void  platformConsoleWriteError(const char * message, u8 colour){

      const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

KINLINE void append_to_log_file(const char * message){

    if(logger_state_ptr && logger_state_ptr->log_file_handle.is_valid){

        u64 length = stringLength(message);
        u64 written = 0;
        if(!filesystem_write(&logger_state_ptr->log_file_handle, length, message, &written)){

            platformConsoleWriteError("ERROR writing to console.log", LOG_LEVEL_ERROR);
        }

    }
}



KINLINE b8 initializeLogging(u64 *memory_requirement, void * state){
    *memory_requirement  = sizeof(logger_system_state); 
    if(state == 0){

        return true;
    };

    logger_state_ptr = (logger_system_state *)state;
    if(!filesystem_open("console.log", FILE_MODE_WRITE, false,  &logger_state_ptr->log_file_handle)){

        platformConsoleWriteError("ERROR: unable to open console.log for writing", LOG_LEVEL_ERROR);
        return false;
    }
    
 

return true;

};

KINLINE void shutdownLogging(void * state){

logger_state_ptr  =0;

}
KINLINE void  platformConsoleWrite(const char * message, u8 colour){
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

KINLINE KAPI void logOutput(logLevel level, const char * message, ...){
const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    
    char out_message[32000];
  kzeroMemory(out_message, sizeof(out_message));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    string_format_v(out_message, message, arg_ptr);
    va_end(arg_ptr);

    string_format(out_message, "%s%s\n", level_strings[level], out_message);   
    // Platform-specific output.
    if (is_error) {
        platformConsoleWriteError(out_message, level);
    } else {
        platformConsoleWrite(out_message, level);
    }

    append_to_log_file(out_message);
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
