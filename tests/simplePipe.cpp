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

template<typename BlockT, bool Ok, typename... ArgsT>
BlockT value(ArgsT &&... args) {
    return abb::success<BlockT>(std::forward<ArgsT>(args)...);
}

template<typename BlockT>
void testSuccessPiping() {
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
    abb::success<BlockT>(7).pipe(
        &Funcs::requireSeven
    ).pipe(
        &Funcs::requireSevenConst
    ).pipe(
        &Funcs::requireSevenConstRef
    ).pipe(
        &Funcs::requireSevenRval
    ).pipe(&Funcs::inc).pipe(&Funcs::inc).pipe(
        std::bind(&Funcs::require, 9, std::placeholders::_1)
    ).run();
}

template<typename BlockT>
class SuccessRefPipingTest {
public:
    SuccessRefPipingTest(): val(10) {}

    void operator()();

private:
    BlockT requireTen(IdentityInt & val) {
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::success<IdentityIntRefBlock>(val);
    };

    IdentityInt val;
};

template<typename BlockT>
void SuccessRefPipingTest<BlockT>::operator()() {
    BlockT block(abb::success(std::ref(this->val)));

    block.pipe(
        std::bind(&SuccessRefPipingTest::requireTen, this, std::placeholders::_1)
    ).run();
}

template<typename BlockT>
void testErrorPiping() {
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
    abb::error<BlockT>(7).pipe(
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
    ).run();
}

template<typename BlockT>
class ErrorRefPipingTest {
public:
    ErrorRefPipingTest(): val(10) {}

    void operator()();

private:
     BlockT requireTen(IdentityInt & val) {
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::error(std::ref(val));
    };

    IdentityInt val;
};

template<typename BlockT>
void ErrorRefPipingTest<BlockT>::operator()() {
    abb::error<BlockT>(this->val).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).run();
}

int main() {
    RUN_FUNCTION(testSuccessPiping<IntBlock>);
    RUN_FUNCTION(ARG(testSuccessPiping<abb::Block<int, void>>));
    RUN_FUNCTION(ARG(testSuccessPiping<abb::Block<int, std::string>>));
    RUN_CLASS(SuccessRefPipingTest<IdentityIntRefBlock>);
    RUN_CLASS(ARG(SuccessRefPipingTest<abb::Block<IdentityInt&, void>>));
    RUN_FUNCTION(ARG(testErrorPiping<abb::ErrorBlock<int>>));
    RUN_FUNCTION(ARG(testErrorPiping<abb::Block<void, int>>));
    RUN_FUNCTION(ARG(testErrorPiping<abb::Block<int, int>>));
    RUN_CLASS(ARG(ErrorRefPipingTest<abb::Block<abb::Und, IdentityInt&>>));
    RUN_CLASS(ARG(ErrorRefPipingTest<abb::Block<IdentityInt&, IdentityInt&>>));
    return 0;
}
