#ifndef ABB_LL_PROXY_BRICK_H
#define ABB_LL_PROXY_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>

#include <memory>

namespace abb {
namespace ll {

template<typename Done>
class ProxyBrick {};

template<typename... Args>
class ProxyBrick<void(Args...)> : public Brick<void(Args...)> {
public:
    typedef ProxyBrick<void(Args...)> ThisType;
    typedef Brick<void(Args...)> ImplType;
    typedef Successor<void(Args...)> SuccessorType;

    ProxyBrick(): block(), successor(nullptr) {}

    virtual ~ProxyBrick() {
        ABB_ASSERT(this->block && this->successor, "Not done yet");
    }

    void setBlock(std::unique_ptr<ImplType> block) {
        ABB_ASSERT(!this->block, "Already got block");
        this->block = std::move(block);
        this->tryFinish();
    }

    virtual void setSuccessor(SuccessorType & successor) {
        ABB_ASSERT(!this->successor, "Already got successor");
        this->successor = &successor;
        this->tryFinish();
    }

private:
    void tryFinish() {
        if (this->block && this->successor) {
            this->block->setSuccessor(*this->successor);
        }
    }

    std::unique_ptr<ImplType> block;
    SuccessorType * successor;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_PROXY_BRICK_H
