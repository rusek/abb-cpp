#include <abb/block.h>

#include <abb/ll/runner.h>

namespace abb {

Handle enqueue(Island & island, BaseBlock<void(), Und> && block) {
    return ll::enqueue(island, ll::unpackBrickPtr(std::move(block)));
}

void enqueueExternal(Island & island, BaseBlock<void(), Und> && block) {
    ll::enqueueExternal(island, ll::unpackBrickPtr(std::move(block)));
}

} // namespace abb
