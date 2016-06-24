#include "helpers/base.h"

#include <vector>
#include <functional>

class Verify123 {
public:
    Verify123(): counter(0) {}
    Verify123(Verify123 const&) = delete;
    Verify123(Verify123 &&) = default;

    abb::void_block operator()(int elem) {
        ++this->counter;
        REQUIRE_EQUAL(this->counter, elem);
        HIT();
        return abb::success();
    }

private:
    int counter;
};

class VerifyUnique123 : Verify123 {
public:
    abb::void_block operator()(std::unique_ptr<int> elem) {
        return this->Verify123::operator()(*elem);
    }
};

std::unique_ptr<int> uniqueInt(int value) {
    return std::unique_ptr<int>(new int(value));
}

abb::void_block testSuccess() {
    EXPECT_HITS(3);

    struct Impl {
        Impl(): elems({1, 2, 3}) {}

        abb::void_block operator()() {
            return abb::each(elems.begin(), elems.end(), Verify123());
        }

        std::vector<int> elems;
    };

    return abb::make<Impl>();
}

abb::void_block testWithMoveIterator() {
    EXPECT_HITS(3);

    struct Impl {
        Impl() {
            this->elems.push_back(uniqueInt(1));
            this->elems.push_back(uniqueInt(2));
            this->elems.push_back(uniqueInt(3));
        }

        abb::void_block operator()() {
            return abb::each(
                std::make_move_iterator(elems.begin()),
                std::make_move_iterator(elems.end()),
                VerifyUnique123()
            );
        }

        std::vector<std::unique_ptr<int>> elems;
    };

    return abb::make<Impl>();
}

abb::void_block testRangeSuccess() {
    EXPECT_HITS(3);

    std::vector<int> elems = {1, 2, 3};
    return abb::each(elems, Verify123());
}

namespace adlns {

struct Movable123 {
    Movable123(): elems({uniqueInt(1), uniqueInt(2), uniqueInt(3)}) {}

    std::unique_ptr<int> elems[3];
};

std::move_iterator<std::unique_ptr<int>*> begin(Movable123 & col) {
    return std::move_iterator<std::unique_ptr<int>*>(col.elems);
}

std::move_iterator<std::unique_ptr<int>*> end(Movable123 & col) {
    return std::move_iterator<std::unique_ptr<int>*>(col.elems + 3);
}

} // namespace adlns

abb::void_block testRangeWithMoveIterator() {
    EXPECT_HITS(3);

    return abb::each(adlns::Movable123(), VerifyUnique123());
}

int main() {
    RUN_FUNCTION(testSuccess);
    RUN_FUNCTION(testWithMoveIterator);
    RUN_FUNCTION(testRangeSuccess);
    RUN_FUNCTION(testRangeWithMoveIterator);
    return 0;
}
