#include "helpers/base.h"

#include <string>


abb::void_block testVoid() {
    EXPECT_HITS(1);

    return abb::error().pipe(abb::und, []() {
        HIT();
    });
}

abb::void_block testInt() {
    EXPECT_HITS(1);

    return abb::error(5).pipe(abb::und, [](int value) {
        REQUIRE_EQUAL(value, 5);
        HIT();
    });
}

abb::void_block testMovable() {
    EXPECT_HITS(1);

    return abb::error(std::unique_ptr<int>(new int(5))).pipe(abb::und, [](std::unique_ptr<int> arg) {
        REQUIRE_EQUAL(*arg, 5);
        HIT();
    });
}

static int refTestInt;

abb::void_block testRef() {
    EXPECT_HITS(1);

    return abb::error(std::ref(refTestInt)).pipe(abb::und, [](int & arg) {
        REQUIRE_EQUAL(&arg, &refTestInt);
        HIT();
    });
}

abb::void_block testMultiArg() {
    EXPECT_HITS(1);

    return abb::error(false, 10, std::string("abc")).pipe(abb::und, [](bool arg1, int arg2, std::string const & arg3) {
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
