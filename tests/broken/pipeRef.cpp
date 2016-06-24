#include <abb.h>

#ifdef APPLY_FIX
#define MUTABLE const
#else
#define MUTABLE
#endif

abb::void_block ignore(int MUTABLE &) {
    return abb::success();
}

void test() {
    abb::success(5).pipe(&ignore);
}
