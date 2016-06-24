#include <abb.h>
#include <vector>

abb::void_block test() {
    std::vector<abb::void_block> blocks;

    return abb::any_of(
#ifdef APPLY_FIX
        std::move
#endif
        (blocks)
    );
}
