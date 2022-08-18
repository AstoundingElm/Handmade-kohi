#include "test_manager.h"
#include "../engine/containers/darray.h"
#include "../engine/clock.inl"

typedef struct test_entry {
    PFN_test func;
    const char* desc;
} test_entry;

global_variable test_entry* tests;

void test_manager_init() {
    tests = (test_entry*)darrayCreate(test_entry);
}

void test_manager_register_test(u8 (*PFN_test)(), const char* desc) {
    test_entry e;
    e.func = PFN_test;
    e.desc = desc;
    darray_push(tests, e);
}

void test_manager_run_tests() {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = darrayLength(tests);

    kclock total_time;
    clockStart(&total_time);

    for (u32 i = 0; i < count; ++i) {
        kclock test_time;
      
      clockStart(&test_time);
    
      
        u8 result = tests[i].func();
    
        clockUpdate(&test_time);
    

        if (result == true) {
            ++passed;
        } else if (result == 2) {
            KWARN("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        } else {
            KERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
       string_format(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        clockUpdate(&total_time);
        KINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    clockStop(&total_time);

    KINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
    
}
