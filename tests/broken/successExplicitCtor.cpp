#include <abb.h>

#ifdef APPLY_FIX
#define EXPLICIT
#else
#define EXPLICIT explicit
#endif

struct A {};

struct B {
    EXPLICIT B(A const&) {}
};

void test() {
    abb::success<abb::block<B>>((A()));
}
