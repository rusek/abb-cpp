#include <abb.h>

abb::VoidBlock ignore(int &) {
    return abb::success();
}

void test() {
    abb::success(5).pipe(&ignore);
}
