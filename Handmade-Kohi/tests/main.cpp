
#include "linearAllocatorTests.h"
int main(){

 test_manager_init();

    // TODO: add test registrations here.
    linear_allocator_register_tests();


    KDEBUG("Starting tests...");

    // Execute tests
    test_manager_run_tests();

    return 0;

}