#ifndef ABB_LL_ABORT_BRICK_H
#define ABB_LL_ABORT_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>
#include <abb/island.h>

namespace abb {
namespace ll {

class AbortBrick : public Brick<Und, Und> {
public:
    AbortBrick() {}

    void abort() {}

    void start(Successor & successor) {
        successor.onUpdate();
    }

    Status getStatus() const {
        return ABORT;
    }
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ABORT_BRICK_H
