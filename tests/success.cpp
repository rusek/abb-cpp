#include "helpers/base.h"

#include <string>


abb::VoidBlock testVoid() {
    EXPECT_HITS(1);

    return abb::success().pipe([]() {
        HIT();
    });
}

abb::VoidBlock testInt() {
    EXPECT_HITS(1);

    return abb::success(5).pipe([](int value) {
        REQUIRE_EQUAL(value, 5);
        HIT();
    });
}

abb::VoidBlock testMovable() {
    EXPECT_HITS(1);

    return abb::success(std::unique_ptr<int>(new int(5))).pipe([](std::unique_ptr<int> arg) {
        REQUIRE_EQUAL(*arg, 5);
        HIT();
    });
}

static int refTestInt;

abb::VoidBlock testRef() {
    EXPECT_HITS(1);

    return abb::success(std::ref(refTestInt)).pipe([](int & arg) {
        REQUIRE_EQUAL(&arg, &refTestInt);
        HIT();
    });
}

abb::VoidBlock testMultiArg() {
    EXPECT_HITS(1);

    return abb::success(false, 10, std::string("abc")).pipe([](bool arg1, int arg2, std::string const & arg3) {
        REQUIRE_EQUAL(arg1, false);
        REQUIRE_EQUAL(arg2, 10);
        REQUIRE_EQUAL(arg3, "abc");
        HIT();
    });
}

int main() {
    RUN_FUNCTION(testVoid);
    RUN_FUNCTION(testInt);
    RUN_FUNCTION(testMovable);
    RUN_FUNCTION(testRef);
    RUN_FUNCTION(testMultiArg);
    return 0;
}
