#ifndef ABB_LL_PROXY_BRICK_H
#define ABB_LL_PROXY_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

namespace internal {

template<typename ResultT, typename ReasonT>
class ProxyBrickBase : public Brick<ResultT, ReasonT> {
public:
    ProxyBrickBase();

    virtual ~ProxyBrickBase();

    void setBrick(BrickPtr<ResultT, ReasonT> brick);

    virtual void setSuccessor(Successor & successor);

protected:
    BrickPtr<ResultT, ReasonT> brick;
    Successor * successor;
};


template<typename ResultT, typename ReasonT>
ProxyBrickBase<ResultT, ReasonT>::ProxyBrickBase(): brick(), successor(nullptr) {}

template<typename ResultT, typename ReasonT>
ProxyBrickBase<ResultT, ReasonT>::~ProxyBrickBase() {
    ABB_ASSERT(this->brick && this->successor, "Not completed yet");
}

template<typename ResultT, typename ReasonT>
void ProxyBrickBase<ResultT, ReasonT>::setBrick(BrickPtr<ResultT, ReasonT> brick) {
    ABB_ASSERT(!this->brick, "Already got brick");
    this->brick = std::move(brick);
    if (this->successor) {
        this->brick.setSuccessor(*this->successor);
    }
}

template<typename ResultT, typename ReasonT>
void ProxyBrickBase<ResultT, ReasonT>::setSuccessor(Successor & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->brick) {
        this->brick.setSuccessor(*this->successor);
    }
}

template<typename ResultT, typename ReasonT>
class ProxyBrickSuccess : public ProxyBrickBase<ResultT, ReasonT> {
public:
    virtual bool hasResult() const;
    virtual ValueToTuple<ResultT> & getResult();
};

template<typename ResultT, typename ReasonT>
bool ProxyBrickSuccess<ResultT, ReasonT>::hasResult() const {
    return this->brick && this->brick.hasResult();
}

template<typename ResultT, typename ReasonT>
ValueToTuple<ResultT> & ProxyBrickSuccess<ResultT, ReasonT>::getResult() {
    return this->brick.getResult();
}

template<typename ReasonT>
class ProxyBrickSuccess<Und, ReasonT> : public ProxyBrickBase<Und, ReasonT> {};


template<typename ResultT, typename ReasonT>
class ProxyBrickError : public ProxyBrickSuccess<ResultT, ReasonT> {
public:
    virtual bool hasReason() const;
    virtual ValueToTuple<ReasonT> & getReason();
};

template<typename ResultT, typename ReasonT>
bool ProxyBrickError<ResultT, ReasonT>::hasReason() const {
    return this->brick && this->brick.hasReason();
}

template<typename ResultT, typename ReasonT>
ValueToTuple<ReasonT> & ProxyBrickError<ResultT, ReasonT>::getReason() {
    return this->brick.getReason();
}

template<typename ResultT>
class ProxyBrickError<ResultT, Und> : public ProxyBrickSuccess<ResultT, Und> {};

} // namespace internal

template<typename ResultT, typename ReasonT>
using ProxyBrick = internal::ProxyBrickError<ResultT, ReasonT>;

} // namespace ll
} // namespace abb

#endif // ABB_LL_PROXY_BRICK_H
