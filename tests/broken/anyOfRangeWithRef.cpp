#include <abb.h>
#include <vector>

abb::VoidBlock test() {
    std::vector<abb::VoidBlock> blocks;

    return abb::anyOf(
#ifdef APPLY_FIX
        std::move
#endif
        (blocks)
    );
}
