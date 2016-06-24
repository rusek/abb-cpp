#include "helpers/base.h"

abb::void_block testSuccess() {
    EXPECT_HITS(3);
    return abb::chain(
        abb::success(),
        abb::make<std::function<abb::void_block()>>([]() {
            HIT(0);
            return abb::success();
        }),
        abb::make<std::function<abb::void_block()>>([]() {
            HIT(1);
            return abb::success();
        }),
        []() {
            HIT(2);
        }
    );
}

abb::void_block testError() {
    typedef abb::block<void, void> block_type;
    EXPECT_HITS(1);
    return abb::chain(
        abb::error<block_type>(),
        abb::make<std::function<abb::void_block()>>([]() {
            FAILURE("should not be called");
            return abb::success();
        })
    ).pipe(abb::pass, []() {
        HIT();
    });
}

abb::void_block testOneArgument() {
    EXPECT_HITS(1);
    return abb::chain(abb::make<std::function<abb::void_block()>>([]() {
        HIT();
        return abb::success();
    }));
}

abb::void_block testVariousResults() {
    return abb::chain(
        abb::success(5),
        [](int val) {
            REQUIRE_EQUAL(val, 5);
            return abb::success(std::string("abc"));
        }, [](std::string val) {
            REQUIRE_EQUAL(val, "abc");
            return abb::success(20, 30);
        }, [](int val1, int val2) {
            REQUIRE_EQUAL(val1, 20);
            REQUIRE_EQUAL(val2, 30);
            return abb::success();
        }
    );
}

int main() {
    RUN_FUNCTION(testSuccess);
    RUN_FUNCTION(testError);
    RUN_FUNCTION(testOneArgument);
    RUN_FUNCTION(testVariousResults);
    return 0;
}
