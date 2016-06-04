#include "helpers/base.h"

#include <functional>

typedef abb::Block<void> VoidBlock;
typedef abb::Block<int> IntBlock;
typedef abb::Block<int&> IntRefBlock;

class IdentityInt {
public:
    explicit IdentityInt(int val): val(val) {}
    IdentityInt(IdentityInt const&) = delete;
    IdentityInt(IdentityInt &&) = delete;
    IdentityInt & operator=(IdentityInt const&) = delete;

    int & operator*() {
        return this->val;
    }

private:
    int val;
};

typedef abb::Block<IdentityInt&> IdentityIntRefBlock;

template<typename BlockT>
VoidBlock testSuccessPiping() {
    EXPECT_HITS(7);

    struct Funcs {
        static BlockT requireSeven(int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static BlockT requireSevenConst(const int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static BlockT requireSevenConstRef(const int & num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static BlockT requireSevenRval(int && num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static BlockT inc(int num) {
            HIT();
            return abb::success(num + 1);
        }
        static BlockT require(int required, int num) {
            HIT();
            REQUIRE_EQUAL(required, num);
            return abb::success(num);
        }
    };
    return abb::success<BlockT>(7).pipe(
        &Funcs::requireSeven
    ).pipe(
        &Funcs::requireSevenConst
    ).pipe(
        &Funcs::requireSevenConstRef
    ).pipe(
        &Funcs::requireSevenRval
    ).pipe(&Funcs::inc).pipe(&Funcs::inc).pipe(
        std::bind(&Funcs::require, 9, std::placeholders::_1)
    ).pipe(IgnoreResult<BlockT>(), IgnoreReason<BlockT>());
}

template<typename BlockT>
class SuccessRefPipingTest {
public:
    SuccessRefPipingTest(): val(10) {}

    VoidBlock operator()();

private:
    BlockT requireTen(IdentityInt & val) {
        HIT();
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::success<IdentityIntRefBlock>(val);
    };

    IdentityInt val;
};

template<typename BlockT>
VoidBlock SuccessRefPipingTest<BlockT>::operator()() {
    EXPECT_HITS(1);
    BlockT block(abb::success(std::ref(this->val)));

    return std::move(block).pipe(
        std::bind(&SuccessRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(IgnoreResult<BlockT>(), IgnoreReason<BlockT>());
}

template<typename BlockT>
VoidBlock testErrorPiping() {
    EXPECT_HITS(4);

    struct Funcs {
        static BlockT requireSeven(int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static BlockT requireSevenConst(const int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static BlockT requireSevenConstRef(const int & num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static BlockT requireSevenRval(int && num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
    };
    return abb::error<BlockT>(7).pipe(
        abb::pass,
        &Funcs::requireSeven
    ).pipe(
        abb::pass,
        &Funcs::requireSevenConst
    ).pipe(
        abb::pass,
        &Funcs::requireSevenConstRef
    ).pipe(
        abb::pass,
        &Funcs::requireSevenRval
    ).pipe(IgnoreResult<BlockT>(), IgnoreReason<BlockT>());
}

template<typename BlockT>
class ErrorRefPipingTest {
public:
    ErrorRefPipingTest(): val(10) {}

    VoidBlock operator()();

private:
     BlockT requireTen(IdentityInt & val) {
        HIT();
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::error(std::ref(val));
    };

    IdentityInt val;
};

template<typename BlockT>
VoidBlock ErrorRefPipingTest<BlockT>::operator()() {
    EXPECT_HITS(2);
    return abb::error<BlockT>(this->val).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(IgnoreResult<BlockT>(), IgnoreReason<BlockT>());
}

abb::VoidBlock testNestedSuccessPiping() {
    EXPECT_HITS(3);

    return abb::success(10).pipe([](int i) {
        HIT();
        REQUIRE_EQUAL(i, 10);
        return abb::success(20).pipe([](int i) {
            HIT();
            REQUIRE_EQUAL(i, 20);
            return abb::success(30);
        });
    }).pipe([](int i) {
        HIT();
        REQUIRE_EQUAL(i, 30);
    });
}

abb::VoidBlock testPipeForwarding() {
    EXPECT_HITS(1);
    return abb::success().pipe([]() {
        return abb::success().pipe([]() {});
    }).pipe(abb::pass).pipe([]() {
        HIT();
    });
}

int main() {
    RUN_FUNCTION(testSuccessPiping<IntBlock>);
    RUN_FUNCTION(testSuccessPiping<abb::Block<int, void>>);
    RUN_FUNCTION(testSuccessPiping<abb::Block<int, std::string>>);
    RUN_CLASS(SuccessRefPipingTest<IdentityIntRefBlock>);
    RUN_CLASS(SuccessRefPipingTest<abb::Block<IdentityInt&, void>>);
    RUN_FUNCTION(testErrorPiping<abb::Block<abb::Und, int>>);
    RUN_FUNCTION(testErrorPiping<abb::Block<void, int>>);
    RUN_FUNCTION(testErrorPiping<abb::Block<int, int>>);
    RUN_CLASS(ErrorRefPipingTest<abb::Block<abb::Und, IdentityInt&>>);
    RUN_CLASS(ErrorRefPipingTest<abb::Block<IdentityInt&, IdentityInt&>>);
    RUN_FUNCTION(testNestedSuccessPiping);
    RUN_FUNCTION(testPipeForwarding);
    return 0;
}
