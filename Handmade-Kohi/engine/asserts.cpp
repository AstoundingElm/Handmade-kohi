#include "/home/petermiller/Desktop/Handmade-Kohi/defines.h"
#include "logger.h"
#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

static inline void reportAssertionFailure(const char * expression, const char * message, const char * file, i32 line){
logOutput(LOG_LEVEL_FATAL, "Assertion failure: %s message: %s in file: %s line: %d", expression, message, file, line );

}

#define KASSERT(expr)                                           \
{                                                               \
    if(expr){                                                   \
    }else{                                                      \
        reportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
        debugBreak();                                           \
    };                                                          \
};                                                              \

//todo: macro madness ## concatenation macro factory for these defines?

#define KASSERT_MSG(expr, message)                                           \
{                                                               \
    if(expr){                                                   \
    }else{                                                      \
        reportAssertionFailure(#expr, message, __FILE__, __LINE__);  \
        debugBreak();                                           \
    };                                                          \
};  

#ifdef _DEBUG
#define KASSERT_DEBUG(expr)                                           \
{                                                               \
    if(expr){                                                   \
    }else{                                                      \
        reportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
        debugBreak();                                           \
    };                                                          \
};  
#else
#define KASSERT_DEBUG(expr)
#endif

#else
#define KASSERT(expr)
#define KASSERT_MSG(expr, message)
#define KASSERT_DEBUG(expr)
#endif
