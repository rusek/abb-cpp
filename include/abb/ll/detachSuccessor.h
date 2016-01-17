#ifndef ABB_LL_DETACH_SUCCESSOR_H
#define ABB_LL_DETACH_SUCCESSOR_H

#include <abb/ll/successor.h>
#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/und.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class DetachSuccessor : private Successor {
public:
    DetachSuccessor(BrickPtr<ResultT, ReasonT> brick);

private:
    virtual void oncomplete();

    BrickPtr<ResultT, ReasonT> brick;
};

template<typename ResultT, typename ReasonT>
DetachSuccessor<ResultT, ReasonT>::DetachSuccessor(
    BrickPtr<ResultT, ReasonT> brick
):
    brick(std::move(brick))
{
    this->brick.setSuccessor(*this);
}

template<typename ResultT, typename ReasonT>
void DetachSuccessor<ResultT, ReasonT>::oncomplete() {
    delete this;
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_DETACH_SUCCESSOR_H
