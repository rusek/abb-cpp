#include <abb/block.h>

namespace abb {

handle enqueue(island & target, base_block<void(), und_t> && block) {
    return ll::enqueue(target, ll::unpack_brick_ptr(std::move(block)));
}

void enqueue_external(island & target, base_block<void(), und_t> && block) {
    ll::enqueue_external(target, ll::unpack_brick_ptr(std::move(block)));
}

} // namespace abb
