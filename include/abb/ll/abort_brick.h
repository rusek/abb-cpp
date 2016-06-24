#ifndef ABB_LL_ABORT_BRICK_H
#define ABB_LL_ABORT_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>
#include <abb/island.h>

namespace abb {
namespace ll {

class abort_brick : public brick<und_t, und_t> {
public:
    abort_brick() {}

    void abort() {}

    void start(successor & succ) {
        succ.on_update();
    }

    status get_status() const {
        return abort_status;
    }
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ABORT_BRICK_H
