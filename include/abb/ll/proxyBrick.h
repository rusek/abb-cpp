#ifndef ABB_LL_PROXY_BRICK_H
#define ABB_LL_PROXY_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>

#include <memory>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class ProxyBrick : public Brick<Result, Reason> {
public:
    typedef Brick<Result, Reason> BaseType;
    typedef Successor<Result, Reason> SuccessorType;

    ProxyBrick();

    virtual ~ProxyBrick();

    void setBrick(std::unique_ptr<BaseType> brick);

    virtual void setSuccessor(SuccessorType & successor);

private:
    std::unique_ptr<BaseType> brick;
    SuccessorType * successor;
};


template<typename Result, typename Reason>
ProxyBrick<Result, Reason>::ProxyBrick(): brick(), successor(nullptr) {}

template<typename Result, typename Reason>
ProxyBrick<Result, Reason>::~ProxyBrick() {
    ABB_ASSERT(this->brick && this->successor, "Not completed yet");
}

template<typename Result, typename Reason>
void ProxyBrick<Result, Reason>::setBrick(std::unique_ptr<BaseType> brick) {
    ABB_ASSERT(!this->brick, "Already got brick");
    this->brick = std::move(brick);
    if (this->successor) {
        this->brick->setSuccessor(*this->successor);
    }
}

template<typename Result, typename Reason>
void ProxyBrick<Result, Reason>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->brick) {
        this->brick->setSuccessor(*this->successor);
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PROXY_BRICK_H
