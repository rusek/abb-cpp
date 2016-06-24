#include "helpers/base.h"


class MakeTester {
public:
    MakeTester(): i(0) {
        HIT(0);
    }

    MakeTester(MakeTester const&) = delete;

    ~MakeTester() {
        HIT(4);
    }

    abb::void_block operator()();
private:
    std::uint32_t i;
};

abb::void_block MakeTester::operator()() {
    this->i++;
    HIT(this->i);
    if (i == 3) {
        return abb::success();
    } else {
        return abb::success().pipe(std::ref(*this));
    }
}

abb::void_block testDestroyedAfterCompletion() {
    EXPECT_HITS(6);

    return abb::make<MakeTester>().pipe([]() {
        HIT(5);
    });
}

int main() {
    RUN_FUNCTION(testDestroyedAfterCompletion);
    return 0;
}
