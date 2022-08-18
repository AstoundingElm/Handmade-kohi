#include "../defines.h"
#include "logger.h"
#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

KINLINE void reportAssertionFailure(const char * expression, const char * message, const char * file, i32 line){
logOutput(LOG_LEVEL_FATAL, "Assertion failure: %s message: %s in file: %s line: %d", expression, message, file, line );

}

#define KASSERT(expr, ...)                                           \
{                                                               \
    if(expr){                                                   \
    }else{                                                      \
        reportAssertionFailure(#expr, #__VA_ARGS__, __FILE__, __LINE__);  \
        debugBreak();                                           \
    };                                                          \
};                                                              \

#endif