#ifndef ABB_LL_PROXY_BRICK_H
#define ABB_LL_PROXY_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>

#include <memory>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class ProxyBrick : public Brick<ResultT, ReasonT> {
public:
    typedef Brick<ResultT, ReasonT> BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

    ProxyBrick();

    virtual ~ProxyBrick();

    void setBrick(std::unique_ptr<BrickType> brick);

    virtual void setSuccessor(SuccessorType & successor);

private:
    std::unique_ptr<BrickType> brick;
    SuccessorType * successor;
};


template<typename ResultT, typename ReasonT>
ProxyBrick<ResultT, ReasonT>::ProxyBrick(): brick(), successor(nullptr) {}

template<typename ResultT, typename ReasonT>
ProxyBrick<ResultT, ReasonT>::~ProxyBrick() {
    ABB_ASSERT(this->brick && this->successor, "Not completed yet");
}

template<typename ResultT, typename ReasonT>
void ProxyBrick<ResultT, ReasonT>::setBrick(std::unique_ptr<BrickType> brick) {
    ABB_ASSERT(!this->brick, "Already got brick");
    this->brick = std::move(brick);
    if (this->successor) {
        this->brick->setSuccessor(*this->successor);
    }
}

template<typename ResultT, typename ReasonT>
void ProxyBrick<ResultT, ReasonT>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->brick) {
        this->brick->setSuccessor(*this->successor);
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PROXY_BRICK_H
