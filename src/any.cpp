#include <abb/any.h>
#include <abb/block.h>

namespace abb {

und_block any() {
    return ll::pack_brick<ll::hold_brick>();
}

} // namespace abb
