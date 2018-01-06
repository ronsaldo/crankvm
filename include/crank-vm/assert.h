#ifndef CRANK_VM_ASSERT_H
#define CRANK_VM_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define crankvm_assertAlways(x) do {\
    if(!(x)) {\
        fprintf(stderr, "CrankVM important assertion \"" #x "\" failed in %s at %s:%d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
    }\
} while(1)

#endif //CRANK_VM_ASSERT_H
