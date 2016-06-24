#include <abb/any.h>
#include <abb/block.h>

namespace abb {

und_block hold() {
    return ll::pack_brick<ll::any_brick<und_t, und_t>>();
}

} // namespace abb
