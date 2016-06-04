#include "helpers/base.h"

#include <atomic>
#include <vector>

abb::VoidBlock testFirstSuccessReturned() {
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
        abb::VoidBlock operator()() {
            return abb::success();
        }
    };

    {
        abb::VoidBlock block = abb::any(abb::make<Dummy>(), abb::make<Dummy>());
        REQUIRE_EQUAL(Tracked::getNumInstances(), 2);
    }
    REQUIRE_EQUAL(Tracked::getNumInstances(), 0);
}

abb::VoidBlock testAnyOf() {
    EXPECT_HITS(1);

    std::vector<abb::Block<int>> blocks;
    blocks.push_back(abb::success(1));
    blocks.push_back(abb::success(2));
    blocks.push_back(abb::success(3));

    return abb::anyOf(
        std::make_move_iterator(blocks.begin()),
        std::make_move_iterator(blocks.end())
    ).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::VoidBlock testAnyOfRange() {
    EXPECT_HITS(1);

    std::vector<abb::Block<int>> blocks;
    blocks.push_back(abb::success(1));
    blocks.push_back(abb::success(2));
    blocks.push_back(abb::success(3));

    return abb::anyOf(std::move(blocks)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::VoidBlock testAnyOfRangeWithRef() {
    struct EmbarrassinglyMuvableBlocks {
        typedef std::move_iterator<std::vector<abb::Block<int>>::iterator> Iterator;

        Iterator begin() {
            return std::make_move_iterator(raw.begin());
        }

        Iterator end() {
            return std::make_move_iterator(raw.end());
        }

        std::vector<abb::Block<int>> raw;
    };

    EXPECT_HITS(1);

    EmbarrassinglyMuvableBlocks blocks;
    blocks.raw.push_back(abb::success(1));
    blocks.raw.push_back(abb::success(2));
    blocks.raw.push_back(abb::success(3));

    return abb::anyOf(blocks).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

namespace adlns {

struct IntBlocks123 {
    IntBlocks123(): blocks({abb::success(1), abb::success(2), abb::success(3)}) {}

    abb::Block<int> blocks[3];
};

abb::Block<int> * begin(IntBlocks123 & blocks) {
    return blocks.blocks;
}

abb::Block<int> * end(IntBlocks123 & blocks) {
    return blocks.blocks + 3;
}

struct MovableIntBlocks123 : IntBlocks123 {};

std::move_iterator<abb::Block<int> *> begin(MovableIntBlocks123 & blocks) {
    return std::make_move_iterator(blocks.blocks);
}

std::move_iterator<abb::Block<int> *> end(MovableIntBlocks123 & blocks) {
    return std::make_move_iterator(blocks.blocks + 3);
}

} // namespace adlns

abb::VoidBlock testAnyOfRangeADL() {
    EXPECT_HITS(1);

    adlns::IntBlocks123 blocks;

    return abb::anyOf(std::move(blocks)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::VoidBlock testAnyOfRangeWithRefADL() {
    EXPECT_HITS(1);

    adlns::MovableIntBlocks123 blocks;

    return abb::anyOf(blocks).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::VoidBlock testWaitForCompletionOfAll() {
    struct Funcs {
        static void setResult(abb::Reply<void> & reply) {
            HIT(1);
            reply.setResult();
        }

        static void impl(abb::Reply<void> & reply) {
            HIT(0);
            REQUIRE_EQUAL(reply.isAborted(), true);
            reply.getIsland().enqueue(std::bind(&Funcs::setResult, std::ref(reply)));
        }
    };

    EXPECT_HITS(3);

    return abb::any(
        abb::success(),
        abb::impl<abb::VoidBlock>(&Funcs::impl)
    ).pipe([]() {
        HIT(2);
    });
}

abb::VoidBlock testWithOneBlock() {
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
