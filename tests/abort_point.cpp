#include "helpers/base.h"

#include <memory>

abb::void_block test_without_abort() {
    EXPECT_HITS(1);
    return abb::abort_point().pipe([]() {
        HIT();
    });
}

void test_with_external_abort(abb::island & island) {
    EXPECT_HITS(1);

    std::shared_ptr<abb::handle> handle(new abb::handle(island.enqueue(
        abb::success().pipe([]() {
            HIT();
        }).pipe(
            abb::abort_point()
        ).pipe([]() {
            FAILURE("computation should be aborted by now");
        })
    )));

    island.enqueue_external(std::bind(&abb::handle::abort, handle));
}

int main() {
    RUN_FUNCTION(test_without_abort);
    RUN_FUNCTION(test_with_external_abort);
    return 0;
}
