#include <abb/any.h>
#include <abb/block.h>

namespace abb {

UndBlock hold() {
    return ll::packBrick<ll::AnyBrick<Und, Und>>();
}

} // namespace abb
