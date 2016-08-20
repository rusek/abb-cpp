#include "helpers/base.h"
#include "helpers/block_mock.h"

abb::void_block test_void_success() {
    EXPECT_HITS(3);
    HIT(0);
    abb::void_block block = abb::impl<abb::void_block>([](abb::reply<void> reply) {
        HIT(2);
        reply.set_result();
    });
    HIT(1);
    return std::move(block);
}

abb::void_block test_int_success() {
    EXPECT_HITS(3);
    return abb::impl<abb::block<int>>([](abb::reply<int> reply) {
        HIT(0);
        reply.set_result(5);
        HIT(1);
    }).pipe([](int value) {
        HIT(2);
        REQUIRE_EQUAL(value, 5);
    });
}

abb::void_block test_ref_success() {
    static int var = 0;
    EXPECT_HITS(1);
    return abb::impl<abb::block<int&>>([](abb::reply<int&> reply) {
        reply.set_result(var);
    }).pipe([](int & value) {
        REQUIRE_EQUAL(&value, &var);
        HIT();
    });
}

abb::void_block test_void_error() {
    typedef abb::block<abb::und_t, void> block_type;

    return abb::impl<block_type>([](abb::get_reply_t<block_type> reply) {
        reply.set_reason();
    }).pipe(abb::und, IgnoreReason<block_type>());
}


abb::void_block test_int_error() {
    typedef abb::block<abb::und_t, int> block_type;
    EXPECT_HITS(3);
    return abb::impl<block_type>([](abb::get_reply_t<block_type> reply) {
        HIT(0);
        reply.set_reason(5);
        HIT(1);
    }).pipe(abb::pass, [](int value) {
        HIT(2);
        REQUIRE_EQUAL(value, 5);
    });
}

abb::void_block test_ref_error() {
    static int var = 0;
    EXPECT_HITS(1);
    return abb::impl<abb::block<abb::und_t, int&>>([](abb::reply<abb::und_t, int&> reply) {
        reply.set_reason(var);
    }).pipe(abb::und, [](int & value) {
        REQUIRE_EQUAL(&value, &var);
        HIT();
    });
}

template<bool SuccessV>
abb::void_block test_mixed() {
    typedef abb::block<int, bool> block_type;
    EXPECT_HITS(2);
    return abb::impl<block_type>([](abb::get_reply_t<block_type> reply) {
        HIT(0);
        if (SuccessV) {
            reply.set_result(10);
        } else {
            reply.set_reason(false);
        }
    }).pipe([](int value) {
        HIT(1);
        REQUIRE_EQUAL(value, 10);
    }, [](bool value) {
        HIT(1);
        REQUIRE_EQUAL(value, false);
    });
}

void enqueue_abort(abb::island & island, abb::handle handle) {
    island.enqueue(abb::partial(&abb::handle::abort, std::move(handle)));
}

enum class to_set {
    result, reason, aborted
};

enum class when {
    now, later
};

template<to_set ToSet, when When>
void test_set_on_abort(abb::island & island) {
    typedef abb::block<int, bool> block_type;
    typedef abb::reply<int, bool> reply_type;

    EXPECT_HITS(ToSet == to_set::aborted ? 2 : 3);

    struct worker {
        std::function<void()> operator()(reply_type reply) {
            HIT(0);
            REQUIRE_EQUAL(reply.is_aborted(), false);
            this->reply = std::move(reply);
            return std::bind(&worker::abort, this);
        }

        void abort() {
            HIT(1);
            REQUIRE_EQUAL(this->reply.is_aborted(), true);
            std::function<void()> setter;
            if (ToSet == to_set::result) {
                setter = std::bind(&reply_type::set_result, &this->reply, 10);
            } else if (ToSet == to_set::reason) {
                setter = std::bind(&reply_type::set_reason, &this->reply, true);
            } else {
                setter = std::bind(&reply_type::set_aborted, &this->reply);
            }

            if (When == when::now) {
                setter();
            } else {
                this->reply.get_island().enqueue(setter);
            }
        }

        reply_type reply;
    };

    abb::void_block block = abb::impl<block_type>(worker()).pipe([](int value) {
        HIT(2);
        REQUIRE(ToSet == to_set::result);
        REQUIRE_EQUAL(value, 10);
    }, [](bool value) {
        HIT(2);
        REQUIRE(ToSet == to_set::reason);
        REQUIRE_EQUAL(value, true);
    });

    enqueue_abort(island, island.enqueue(std::move(block)));
}

void test_movable_abort_notification(abb::island & island) {
    EXPECT_HITS(2);

    struct movable_function {
        explicit movable_function(std::function<void()> func): func(func) {}
        movable_function(movable_function &&) = default;
        movable_function(movable_function const&) = delete;

        void operator()() {
            this->func();
        }

        std::function<void()> func;
    };

    struct worker {
        movable_function operator()(abb::und_reply reply) {
            HIT(0);
            REQUIRE_EQUAL(reply.is_aborted(), false);
            this->reply = std::move(reply);
            return movable_function(std::bind(&worker::abort, this));
        }

        void abort() {
            HIT(1);
            REQUIRE_EQUAL(this->reply.is_aborted(), true);
            this->reply.set_aborted();
        }

        abb::und_reply reply;
    };

    abb::void_block block = abb::impl<abb::und_block>(worker());

    enqueue_abort(island, island.enqueue(std::move(block)));
}

abb::void_block test_no_enqueue_on_immediate_success() {
    EXPECT_HITS(3);

    return abb::impl<abb::void_block>([](abb::void_reply reply) {
        HIT(0);
        reply.get_island().enqueue([]() {
            HIT(2);
        });
        reply.set_result();
    }).pipe([]() {
        HIT(1);
    });
}

abb::void_block test_no_enqueue_on_immediate_error() {
    EXPECT_HITS(3);

    return abb::impl<abb::block<abb::und_t, void>>([](abb::reply<abb::und_t, void> reply) {
        HIT(0);
        reply.get_island().enqueue([]() {
            HIT(2);
        });
        reply.set_reason();
    }).pipe(abb::pass, []() {
        HIT(1);
    });
}

void test_no_enqueue_on_immediate_abort(abb::island & island) {
    static abb::handle handle;

    EXPECT_HITS(3);

    block_mock<void> partner;
    partner.expect_start();
    partner.expect_abort([partner]() {
        HIT(1);
        partner.set_aborted();
    });

    struct abort_worker {
        std::function<void()> operator()(abb::void_reply reply) {
            this->reply = std::move(reply);
            handle.abort();
            return std::bind(&abort_worker::abort, this);
        }

        void abort() {
            HIT(0);
            this->reply.get_island().enqueue([]() {
                HIT(2);
            });
            this->reply.set_aborted();
        }

        abb::void_reply reply;
    };

    EXPECT_HITS(3);

    abb::void_block impl_block = abb::impl<abb::void_block>(abort_worker());

    handle = island.enqueue(abb::any(std::move(impl_block), partner.get()));
}

int main() {
    RUN_FUNCTION(test_void_success);
    RUN_FUNCTION(test_int_success);
    RUN_FUNCTION(test_ref_success);
    RUN_FUNCTION(test_void_error);
    RUN_FUNCTION(test_int_error);
    RUN_FUNCTION(test_ref_error);
    RUN_FUNCTION(test_mixed<false>);
    RUN_FUNCTION(test_mixed<true>);
    RUN_FUNCTION(test_set_on_abort<to_set::result, when::now>);
    RUN_FUNCTION(test_set_on_abort<to_set::result, when::later>);
    RUN_FUNCTION(test_set_on_abort<to_set::reason, when::now>);
    RUN_FUNCTION(test_set_on_abort<to_set::reason, when::later>);
    RUN_FUNCTION(test_set_on_abort<to_set::aborted, when::now>);
    RUN_FUNCTION(test_set_on_abort<to_set::aborted, when::later>);
    RUN_FUNCTION(test_movable_abort_notification);
    RUN_FUNCTION(test_no_enqueue_on_immediate_success);
    RUN_FUNCTION(test_no_enqueue_on_immediate_error);
    RUN_FUNCTION(test_no_enqueue_on_immediate_abort);
    return 0;
}
