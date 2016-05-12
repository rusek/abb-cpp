#ifndef ABB_LL_DETACH_SUCCESSOR_H
#define ABB_LL_DETACH_SUCCESSOR_H

#include <abb/ll/successor.h>
#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/und.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class DetachSuccessor : private Successor, private Task {
public:
    DetachSuccessor(BrickPtr<ResultT, ReasonT> brick);

private:
    virtual void oncomplete();
    virtual Island & getIsland() const;
    virtual void run();

    BrickPtr<ResultT, ReasonT> brick;
};

template<typename ResultT, typename ReasonT>
DetachSuccessor<ResultT, ReasonT>::DetachSuccessor(
    BrickPtr<ResultT, ReasonT> brick
):
    brick(std::move(brick))
{
    Island::current().enqueue(*this);
}

template<typename ResultT, typename ReasonT>
void DetachSuccessor<ResultT, ReasonT>::oncomplete() {
    delete this;
}

template<typename ResultT, typename ReasonT>
Island & DetachSuccessor<ResultT, ReasonT>::getIsland() const {
    return Island::current();
}

template<typename ResultT, typename ReasonT>
void DetachSuccessor<ResultT, ReasonT>::run() {
    this->brick.run(*this);
}



} // namespace ll
} // namespace abb

#endif // ABB_LL_DETACH_SUCCESSOR_H
