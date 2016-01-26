#ifndef ABB_LL_PURE_EXIT_BRICK_H
#define ABB_LL_PURE_EXIT_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <functional>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class PureExitBrick : public Brick<ResultT, ReasonT>, private Successor {
public:
    PureExitBrick(std::function<void()> onexit, BrickPtr<ResultT, ReasonT> brick);

    virtual ~PureExitBrick();

    void setSuccessor(Successor & successor);

    Status getStatus() const {
        return this->brick.getStatus();
    }

    ValueToTuple<ResultT> & getResult();

private:
    virtual void oncomplete();

    std::function<void()> onexit;
    BrickPtr<ResultT, ReasonT> brick;
    Successor * successor;
};

template<typename ResultT, typename ReasonT>
PureExitBrick<ResultT, ReasonT>::PureExitBrick(std::function<void()> onexit, BrickPtr<ResultT, ReasonT> brick):
    onexit(onexit),
    brick(std::move(brick)),
    successor(nullptr) {}

template<typename ResultT, typename ReasonT>
PureExitBrick<ResultT, ReasonT>::~PureExitBrick() {}

template<typename ResultT, typename ReasonT>
void PureExitBrick<ResultT, ReasonT>::setSuccessor(Successor & successor) {
    this->successor = &successor;
    this->brick.setSuccessor(*this); // FIXME too late for that!
}

template<typename ResultT, typename ReasonT>
ValueToTuple<ResultT> & PureExitBrick<ResultT, ReasonT>::getResult() {
    return this->brick.getResult();
}

template<typename ResultT, typename ReasonT>
void PureExitBrick<ResultT, ReasonT>::oncomplete() {
    this->onexit();
    this->successor->oncomplete();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PURE_EXIT_BRICK_H
