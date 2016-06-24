#include "helpers/base.h"

abb::void_block testVoidSuccess() {
    EXPECT_HITS(3);
    HIT(0);
    abb::void_block block = abb::impl<abb::void_block>([](abb::reply<void> & reply) {
        HIT(2);
        reply.set_result();
    });
    HIT(1);
    return std::move(block);
}

abb::void_block testIntSuccess() {
    EXPECT_HITS(3);
    return abb::impl<abb::block<int>>([](abb::reply<int> & reply) {
        HIT(0);
        reply.set_result(5);
        HIT(1);
    }).pipe([](int value) {
        HIT(2);
        REQUIRE_EQUAL(value, 5);
    });
}

abb::void_block testRefSuccess() {
    static int var = 0;
    EXPECT_HITS(1);
    return abb::impl<abb::block<int&>>([](abb::reply<int&> & reply) {
        reply.set_result(var);
    }).pipe([](int & value) {
        REQUIRE_EQUAL(&value, &var);
        HIT();
    });
}

abb::void_block testVoidError() {
    typedef abb::block<abb::und_t, void> block_type;

    return abb::impl<block_type>([](abb::get_reply_t<block_type> & reply) {
        reply.set_reason();
    }).pipe(abb::und, IgnoreReason<block_type>());
}


abb::void_block testIntError() {
    typedef abb::block<abb::und_t, int> block_type;
    EXPECT_HITS(3);
    return abb::impl<block_type>([](abb::get_reply_t<block_type> & reply) {
        HIT(0);
        reply.set_reason(5);
        HIT(1);
    }).pipe(abb::pass, [](int value) {
        HIT(2);
        REQUIRE_EQUAL(value, 5);
    });
}

abb::void_block testRefError() {
    static int var = 0;
    EXPECT_HITS(1);
    return abb::impl<abb::block<abb::und_t, int&>>([](abb::reply<abb::und_t, int&> & reply) {
        reply.set_reason(var);
    }).pipe(abb::und, [](int & value) {
        REQUIRE_EQUAL(&value, &var);
        HIT();
    });
}

template<bool SuccessV>
abb::void_block testMixed() {
    typedef abb::block<int, bool> block_type;
    EXPECT_HITS(2);
    return abb::impl<block_type>([](abb::get_reply_t<block_type> & reply) {
        HIT(0);
        if (SuccessV) {
            reply.set_result(10);
        } else {
            reply.set_reason(false);
        }
    }).pipe([](int value) {
        HIT(1);
        REQUIRE_EQUAL(value, 10);
    }, [](bool value) {
        HIT(1);
        REQUIRE_EQUAL(value, false);
    });
}

int main() {
    RUN_FUNCTION(testVoidSuccess);
    RUN_FUNCTION(testIntSuccess);
    RUN_FUNCTION(testRefSuccess);
    RUN_FUNCTION(testVoidError);
    RUN_FUNCTION(testIntError);
    RUN_FUNCTION(testRefError);
    RUN_FUNCTION(testMixed<false>);
    RUN_FUNCTION(testMixed<true>);
    return 0;
}
