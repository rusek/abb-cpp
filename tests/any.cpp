#include "helpers/base.h"
#include "helpers/block_mock.h"

#include <atomic>
#include <vector>

abb::void_block test_first_success_returned() {
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


void test_any_disposal() {
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

abb::void_block test_any_of() {
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

abb::void_block test_any_of_range() {
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

abb::void_block test_any_of_range_with_ref() {
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

abb::void_block test_any_of_range_adl() {
    EXPECT_HITS(1);

    adlns::IntBlocks123 blocks;

    return abb::any_of(std::move(blocks)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::void_block test_any_of_range_with_ref_adl() {
    EXPECT_HITS(1);

    adlns::MovableIntBlocks123 blocks;

    return abb::any_of(blocks).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

abb::void_block test_wait_for_completion_of_all() {
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

abb::void_block test_with_one_block() {
    EXPECT_HITS(1);
    return abb::any(abb::success(1)).pipe([](int value) {
        REQUIRE_EQUAL(value, 1);
        HIT();
    });
}

void test_with_no_blocks(abb::island & island) {
    std::shared_ptr<abb::handle> handle(new abb::handle(island.enqueue(
        abb::void_block(abb::any()).pipe([]() {
            FAILURE("should never be called");
        })
    )));

    island.enqueue(std::bind(&abb::handle::abort, handle));
}

void test_any_of_with_no_blocks(abb::island & island) {
    std::vector<abb::void_block> blocks;

    std::shared_ptr<abb::handle> handle(new abb::handle(island.enqueue(
        abb::void_block(abb::any_of(
            std::make_move_iterator(blocks.begin()),
            std::make_move_iterator(blocks.end())
        )).pipe([]() {
            FAILURE("should never be called");
        })
    )));

    island.enqueue(std::bind(&abb::handle::abort, handle));
}

void enqueue_already_aborted(abb::island & island, abb::void_block block) {
    std::shared_ptr<abb::handle> handle(new abb::handle(island.enqueue(
        abb::impl<abb::void_block>([](abb::void_reply & reply) {
            reply.get_island().enqueue(std::bind(&abb::void_reply::set_result, &reply));
        }).pipe(std::move(block))
    )));

    island.enqueue(std::bind(&abb::handle::abort, handle));
}

void test_any_with_no_blocks_already_aborted(abb::island & island) {
    enqueue_already_aborted(
        island,
        abb::void_block(abb::any()).pipe([]() {
            FAILURE("should never be called");
        })
    );
}

void test_any_of_with_no_blocks_already_aborted(abb::island & island) {
    std::vector<abb::void_block> blocks;

    enqueue_already_aborted(
        island,
        abb::void_block(abb::any_of(
            std::make_move_iterator(blocks.begin()),
            std::make_move_iterator(blocks.end())
        )).pipe([]() {
            FAILURE("should never be called");
        })
    );
}

void test_any_already_aborted(abb::island & island) {
    EXPECT_HITS(5);
    block_mock<void> m1, m2;

    m1.expect_start([=]() {
        REQUIRE_EQUAL(m1.is_aborted(), true);
        HIT(0);
        m1.enqueue([=]() {
            HIT(2);
            m1.set_result();
            m2.enqueue([=]() {
                HIT(3);
                m2.set_aborted();
            });
        });
    });
    m2.expect_start([=]() {
        REQUIRE_EQUAL(m2.is_aborted(), true);
        HIT(1);
    });

    enqueue_already_aborted(
        island,
        abb::any(m1.get(), m2.get()).pipe([]() {
            HIT(4);
        })
    );
}

abb::void_block test_remaining_blocks_aborted() {
    block_mock<void> m1, m2, m3;

    m1.expect_start();
    m2.expect_start([=]() {
        m2.enqueue_set_result();
        m1.expect_abort([=]() {
            m1.enqueue([=]() {
                m1.set_aborted();
                m3.enqueue_set_aborted();
            });
        });
        m3.expect_abort();
    });
    m3.expect_start();

    return abb::any(m1.get(), m2.get(), m3.get());
}

int main() {
    RUN_FUNCTION(test_first_success_returned);
    RUN_FUNCTION(test_any_disposal);
    RUN_FUNCTION(test_any_of);
    RUN_FUNCTION(test_any_of_range);
    RUN_FUNCTION(test_any_of_range_with_ref);
    RUN_FUNCTION(test_any_of_range_adl);
    RUN_FUNCTION(test_any_of_range_with_ref_adl);
    RUN_FUNCTION(test_wait_for_completion_of_all);
    RUN_FUNCTION(test_with_one_block);
    RUN_FUNCTION(test_with_no_blocks);
    RUN_FUNCTION(test_any_of_with_no_blocks);
    RUN_FUNCTION(test_any_with_no_blocks_already_aborted);
    RUN_FUNCTION(test_any_already_aborted);
    RUN_FUNCTION(test_remaining_blocks_aborted);
    return 0;
}
