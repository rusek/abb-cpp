#include "helpers/base.h"

abb::VoidBlock testVoidSuccess() {
    EXPECT_HITS(3);
    HIT(0);
    abb::VoidBlock block = abb::impl<abb::VoidBlock>([](abb::Reply<void> & reply) {
        HIT(2);
        reply.setResult();
    });
    HIT(1);
    return std::move(block);
}

abb::VoidBlock testIntSuccess() {
    EXPECT_HITS(3);
    return abb::impl<abb::Block<int>>([](abb::Reply<int> & reply) {
        HIT(0);
        reply.setResult(5);
        HIT(1);
    }).pipe([](int value) {
        HIT(2);
        REQUIRE_EQUAL(value, 5);
    });
}

abb::VoidBlock testRefSuccess() {
    static int var = 0;
    EXPECT_HITS(1);
    return abb::impl<abb::Block<int&>>([](abb::Reply<int&> & reply) {
        reply.setResult(var);
    }).pipe([](int & value) {
        REQUIRE_EQUAL(&value, &var);
        HIT();
    });
}

abb::VoidBlock testVoidError() {
    typedef abb::Block<abb::Und, void> BlockType;

    return abb::impl<BlockType>([](abb::GetReply<BlockType> & reply) {
        reply.setReason();
    }).pipe(abb::und, IgnoreReason<BlockType>());
}


abb::VoidBlock testIntError() {
    typedef abb::Block<abb::Und, int> BlockType;
    EXPECT_HITS(3);
    return abb::impl<BlockType>([](abb::GetReply<BlockType> & reply) {
        HIT(0);
        reply.setReason(5);
        HIT(1);
    }).pipe(abb::pass, [](int value) {
        HIT(2);
        REQUIRE_EQUAL(value, 5);
    });
}

abb::VoidBlock testRefError() {
    static int var = 0;
    EXPECT_HITS(1);
    return abb::impl<abb::Block<abb::Und, int&>>([](abb::Reply<abb::Und, int&> & reply) {
        reply.setReason(var);
    }).pipe(abb::und, [](int & value) {
        REQUIRE_EQUAL(&value, &var);
        HIT();
    });
}

template<bool SuccessV>
abb::VoidBlock testMixed() {
    typedef abb::Block<int, bool> BlockType;
    EXPECT_HITS(2);
    return abb::impl<BlockType>([](abb::GetReply<BlockType> & reply) {
        HIT(0);
        if (SuccessV) {
            reply.setResult(10);
        } else {
            reply.setReason(false);
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
