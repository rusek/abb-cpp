#ifndef ABB_LL_DETACH_SUCCESSOR_H
#define ABB_LL_DETACH_SUCCESSOR_H

#include <abb/ll/successor.h>
#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/special.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT, bool External>
class DetachSuccessor : private Successor, private Task {
public:
    DetachSuccessor(Island & island, BrickPtr<ResultT, ReasonT> brick);

private:
    virtual void oncomplete();
    virtual Island & getIsland() const;
    virtual void run();

    Island & island;
    BrickPtr<ResultT, ReasonT> brick;
};

template<typename ResultT, typename ReasonT, bool External>
DetachSuccessor<ResultT, ReasonT, External>::DetachSuccessor(
    Island & island,
    BrickPtr<ResultT, ReasonT> brick
):
    island(island),
    brick(std::move(brick))
{
    this->island.increfExternal();
    if (External) {
        this->island.enqueueExternal(static_cast<Task&>(*this));
    } else {
        this->island.enqueue(static_cast<Task&>(*this));
    }
}

template<typename ResultT, typename ReasonT, bool External>
void DetachSuccessor<ResultT, ReasonT, External>::oncomplete() {
    this->island.decrefExternal();
    delete this;
}

template<typename ResultT, typename ReasonT, bool External>
Island & DetachSuccessor<ResultT, ReasonT, External>::getIsland() const {
    return this->island;
}

template<typename ResultT, typename ReasonT, bool External>
void DetachSuccessor<ResultT, ReasonT, External>::run() {
    this->brick.start(*this);
}



} // namespace ll
} // namespace abb

#endif // ABB_LL_DETACH_SUCCESSOR_H
