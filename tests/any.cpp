#include "helpers/base.h"

#include <atomic>
#include <vector>

abb::void_block testFirstSuccessReturned() {
    EXPECT_HITS(1);
    return abb::any(abb::success(1), abb::success(2)).pipe([](int value) {
        HIT();
        REQUIRE_EQUAL(value, 1);
    });
}

class Tracked {
public:
    Tracked(Tracked const&) {
        ++Tracked::numInstances;
    }
    Tracked(Tracked &&) {
        ++Tracked::numInstances;
    }
    Tracked() {
        ++Tracked::numInstances;
    }

    ~Tracked() {
        --Tracked::numInstances;
    }

    static std::size_t getNumInstances() {
        return Tracked::numInstances;
    }

private:
    static std::atomic_size_t numInstances;
};

std::atomic_size_t Tracked::numInstances;


void testAnyDisposal() {
    struct Dummy : Tracked {
        abb::void_block operator()() {
            return abb::success();
        }
    };

    {
        abb::void_block block = abb::any(abb::make<Dummy>(), abb::make<Dummy>());
        REQUIRE_EQUAL(Tracked::getNumInstances(), 2);
    }
    REQUIRE_EQUAL(Tracked::getNumInstances(), 0);
}

abb::void_block testAnyOf() {
    EXPECT_HITS(1);

    std::vector<abb::block<int>> blocks;
    blocks.push_back(abb::success(1));
    blocks.push_back(abb::success(2));
    blocks.push_back(abb::success(3));

    return abb::any_of(
        std::make_move_iterator(blocks.begin()),
        std::make_move_iterator(blocks.end())
    ).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::void_block testAnyOfRange() {
    EXPECT_HITS(1);

    std::vector<abb::block<int>> blocks;
    blocks.push_back(abb::success(1));
    blocks.push_back(abb::success(2));
    blocks.push_back(abb::success(3));

    return abb::any_of(std::move(blocks)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::void_block testAnyOfRangeWithRef() {
    struct EmbarrassinglyMuvableBlocks {
        typedef std::move_iterator<std::vector<abb::block<int>>::iterator> Iterator;

        Iterator begin() {
            return std::make_move_iterator(raw.begin());
        }

        Iterator end() {
            return std::make_move_iterator(raw.end());
        }

        std::vector<abb::block<int>> raw;
    };

    EXPECT_HITS(1);

    EmbarrassinglyMuvableBlocks blocks;
    blocks.raw.push_back(abb::success(1));
    blocks.raw.push_back(abb::success(2));
    blocks.raw.push_back(abb::success(3));

    return abb::any_of(blocks).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

namespace adlns {

struct IntBlocks123 {
    IntBlocks123(): blocks({abb::success(1), abb::success(2), abb::success(3)}) {}

    abb::block<int> blocks[3];
};

abb::block<int> * begin(IntBlocks123 & blocks) {
    return blocks.blocks;
}

abb::block<int> * end(IntBlocks123 & blocks) {
    return blocks.blocks + 3;
}

struct MovableIntBlocks123 : IntBlocks123 {};

std::move_iterator<abb::block<int> *> begin(MovableIntBlocks123 & blocks) {
    return std::make_move_iterator(blocks.blocks);
}

std::move_iterator<abb::block<int> *> end(MovableIntBlocks123 & blocks) {
    return std::make_move_iterator(blocks.blocks + 3);
}

} // namespace adlns

abb::void_block testAnyOfRangeADL() {
    EXPECT_HITS(1);

    adlns::IntBlocks123 blocks;

    return abb::any_of(std::move(blocks)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::void_block testAnyOfRangeWithRefADL() {
    EXPECT_HITS(1);

    adlns::MovableIntBlocks123 blocks;

    return abb::any_of(blocks).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::void_block testWaitForCompletionOfAll() {
    struct Funcs {
        static void set_result(abb::reply<void> & reply) {
            HIT(1);
            reply.set_result();
        }

        static void impl(abb::reply<void> & reply) {
            HIT(0);
            REQUIRE_EQUAL(reply.is_aborted(), true);
            reply.get_island().enqueue(std::bind(&Funcs::set_result, std::ref(reply)));
        }
    };

    EXPECT_HITS(3);

    return abb::any(
        abb::success(),
        abb::impl<abb::void_block>(&Funcs::impl)
    ).pipe([]() {
        HIT(2);
    });
}

abb::void_block testWithOneBlock() {
    EXPECT_HITS(1);
    return abb::any(abb::success(1)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}


int main() {
    RUN_FUNCTION(testFirstSuccessReturned);
    RUN_FUNCTION(testAnyDisposal);
    RUN_FUNCTION(testAnyOf);
    RUN_FUNCTION(testAnyOfRange);
    RUN_FUNCTION(testAnyOfRangeWithRef);
    RUN_FUNCTION(testAnyOfRangeADL);
    RUN_FUNCTION(testAnyOfRangeWithRefADL);
    RUN_FUNCTION(testWaitForCompletionOfAll);
    RUN_FUNCTION(testWithOneBlock);
    return 0;
}
