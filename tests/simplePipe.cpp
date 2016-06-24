#include "helpers/base.h"

#include <functional>

typedef abb::block<void> void_block;
typedef abb::block<int> IntBlock;
typedef abb::block<int&> IntRefBlock;

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

typedef abb::block<IdentityInt&> IdentityIntRefBlock;

template<typename Block>
void_block testSuccessPiping() {
    EXPECT_HITS(7);

    struct Funcs {
        static Block requireSeven(int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static Block requireSevenConst(const int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static Block requireSevenConstRef(const int & num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static Block requireSevenRval(int && num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static Block inc(int num) {
            HIT();
            return abb::success(num + 1);
        }
        static Block require(int required, int num) {
            HIT();
            REQUIRE_EQUAL(required, num);
            return abb::success(num);
        }
    };
    return abb::success<Block>(7).pipe(
        &Funcs::requireSeven
    ).pipe(
        &Funcs::requireSevenConst
    ).pipe(
        &Funcs::requireSevenConstRef
    ).pipe(
        &Funcs::requireSevenRval
    ).pipe(&Funcs::inc).pipe(&Funcs::inc).pipe(
        std::bind(&Funcs::require, 9, std::placeholders::_1)
    ).pipe(IgnoreResult<Block>(), IgnoreReason<Block>());
}

template<typename Block>
class SuccessRefPipingTest {
public:
    SuccessRefPipingTest(): val(10) {}

    void_block operator()();

private:
    Block requireTen(IdentityInt & val) {
        HIT();
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::success<IdentityIntRefBlock>(val);
    };

    IdentityInt val;
};

template<typename Block>
void_block SuccessRefPipingTest<Block>::operator()() {
    EXPECT_HITS(1);
    Block block(abb::success(std::ref(this->val)));

    return std::move(block).pipe(
        std::bind(&SuccessRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(IgnoreResult<Block>(), IgnoreReason<Block>());
}

template<typename Block>
void_block testErrorPiping() {
    EXPECT_HITS(4);

    struct Funcs {
        static Block requireSeven(int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static Block requireSevenConst(const int num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static Block requireSevenConstRef(const int & num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static Block requireSevenRval(int && num) {
            HIT();
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
    };
    return abb::error<Block>(7).pipe(
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
    ).pipe(IgnoreResult<Block>(), IgnoreReason<Block>());
}

template<typename Block>
class ErrorRefPipingTest {
public:
    ErrorRefPipingTest(): val(10) {}

    void_block operator()();

private:
     Block requireTen(IdentityInt & val) {
        HIT();
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::error(std::ref(val));
    };

    IdentityInt val;
};

template<typename Block>
void_block ErrorRefPipingTest<Block>::operator()() {
    EXPECT_HITS(2);
    return abb::error<Block>(this->val).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(IgnoreResult<Block>(), IgnoreReason<Block>());
}

abb::void_block testNestedSuccessPiping() {
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

abb::void_block testPipeForwarding() {
    EXPECT_HITS(1);
    return abb::success().pipe([]() {
        return abb::success().pipe([]() {});
    }).pipe(abb::pass).pipe([]() {
        HIT();
    });
}

int main() {
    RUN_FUNCTION(testSuccessPiping<IntBlock>);
    RUN_FUNCTION(testSuccessPiping<abb::block<int, void>>);
    RUN_FUNCTION(testSuccessPiping<abb::block<int, std::string>>);
    RUN_CLASS(SuccessRefPipingTest<IdentityIntRefBlock>);
    RUN_CLASS(SuccessRefPipingTest<abb::block<IdentityInt&, void>>);
    RUN_FUNCTION(testErrorPiping<abb::block<abb::und_t, int>>);
    RUN_FUNCTION(testErrorPiping<abb::block<void, int>>);
    RUN_FUNCTION(testErrorPiping<abb::block<int, int>>);
    RUN_CLASS(ErrorRefPipingTest<abb::block<abb::und_t, IdentityInt&>>);
    RUN_CLASS(ErrorRefPipingTest<abb::block<IdentityInt&, IdentityInt&>>);
    RUN_FUNCTION(testNestedSuccessPiping);
    RUN_FUNCTION(testPipeForwarding);
    return 0;
}
