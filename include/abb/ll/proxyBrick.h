#ifndef ABB_LL_PROXY_BRICK_H
#define ABB_LL_PROXY_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class ProxyBrick : public Brick<ResultT, ReasonT> {
public:
    ProxyBrick();

    virtual ~ProxyBrick();

    void setBrick(BrickPtr<ResultT, ReasonT> brick);

    void setSuccessor(Successor & successor);

    bool hasResult() const {
        return this->brick && this->brick.hasResult();
    }

    ValueToTuple<ResultT> & getResult() {
        return this->brick.getResult();
    }

    bool hasReason() const {
        return this->brick && this->brick.hasReason();
    }

    ValueToTuple<ReasonT> & getReason() {
        return this->brick.getReason();
    }

protected:
    BrickPtr<ResultT, ReasonT> brick;
    Successor * successor;
};


template<typename ResultT, typename ReasonT>
ProxyBrick<ResultT, ReasonT>::ProxyBrick(): brick(), successor(nullptr) {}

template<typename ResultT, typename ReasonT>
ProxyBrick<ResultT, ReasonT>::~ProxyBrick() {
    ABB_ASSERT(this->brick && this->successor, "Not completed yet");
}

template<typename ResultT, typename ReasonT>
void ProxyBrick<ResultT, ReasonT>::setBrick(BrickPtr<ResultT, ReasonT> brick) {
    ABB_ASSERT(!this->brick, "Already got brick");
    this->brick = std::move(brick);
    if (this->successor) {
        this->brick.setSuccessor(*this->successor);
    }
}

template<typename ResultT, typename ReasonT>
void ProxyBrick<ResultT, ReasonT>::setSuccessor(Successor & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->brick) {
        this->brick.setSuccessor(*this->successor);
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PROXY_BRICK_H
