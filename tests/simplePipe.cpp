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

void testSuccessPiping() {
    struct Funcs {
        static IntBlock requireSeven(int num) {
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static IntBlock requireSevenConst(const int num) {
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static IntBlock requireSevenConstRef(const int & num) {
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static IntBlock requireSevenRval(int && num) {
            REQUIRE_EQUAL(num, 7);
            return abb::success(num);
        }
        static IntBlock inc(int num) {
            return abb::success(num + 1);
        }
        static IntBlock require(int required, int num) {
            REQUIRE_EQUAL(required, num);
            return abb::success(num);
        }
    };
    abb::success(7).pipe(
        &Funcs::requireSeven
    ).pipe(
        &Funcs::requireSevenConst
    ).pipe(
        &Funcs::requireSevenConstRef
    ).pipe(
        &Funcs::requireSevenRval
    ).pipe(&Funcs::inc).pipe(&Funcs::inc).pipe(
        std::bind(&Funcs::require, 9, std::placeholders::_1)
    );
}

class SuccessRefPipingTest {
public:
    SuccessRefPipingTest(): val(10) {}

    void operator()();

private:
    IdentityIntRefBlock requireTen(IdentityInt & val) {
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::success<IdentityIntRefBlock>(val);
    };

    IdentityInt val;
};

void SuccessRefPipingTest::operator()() {
    IdentityIntRefBlock block(abb::success(std::ref(this->val)));

    block.pipe(
        std::bind(&SuccessRefPipingTest::requireTen, this, std::placeholders::_1)
    );
}

void testErrorPiping() {
    typedef abb::ErrorBlock<int> BlockType;

    struct Funcs {
        static BlockType requireSeven(int num) {
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static BlockType requireSevenConst(const int num) {
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static BlockType requireSevenConstRef(const int & num) {
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
        static BlockType requireSevenRval(int && num) {
            REQUIRE_EQUAL(num, 7);
            return abb::error(num);
        }
    };
    abb::error(7).pipe(
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
    );
}

class ErrorRefPipingTest {
public:
    ErrorRefPipingTest(): val(10) {}

    void operator()();

private:
     abb::Block<abb::Und, void(IdentityInt&)> requireTen(IdentityInt & val) {
        REQUIRE_EQUAL(&val, &this->val);
        REQUIRE_EQUAL(*val, 10);
        return abb::error<abb::Block<abb::Und, void(IdentityInt&)>>(val);
    };

    IdentityInt val;
};

void ErrorRefPipingTest::operator()() {
    abb::error(std::ref(this->val)).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    ).pipe(
        abb::pass,
        std::bind(&ErrorRefPipingTest::requireTen, this, std::placeholders::_1)
    );
}

int main() {
    RUN_FUNCTION(testSuccessPiping);
    RUN_CLASS(SuccessRefPipingTest);
    RUN_FUNCTION(testErrorPiping);
    RUN_CLASS(ErrorRefPipingTest);
    return 0;
}
