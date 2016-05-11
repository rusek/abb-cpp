#ifndef ABB_LL_ABORT_BRICK_H
#define ABB_LL_ABORT_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>
#include <abb/island.h>

namespace abb {
namespace ll {

class AbortBrick : public Brick<Und, Und>, private Task {
public:
    AbortBrick(): successor(nullptr) {}

    void abort() {}

    void setSuccessor(Successor & successor) {
        ABB_ASSERT(!this->successor, "Already got successor");
        this->successor = &successor;
        Island::current().enqueue(*this);
    }

    Status getStatus() const {
        return ABORT;
    }

private:
    virtual void run();

    Successor * successor;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ABORT_BRICK_H
