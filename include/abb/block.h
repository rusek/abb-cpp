#ifndef ABB_BLOCK_H
#define ABB_BLOCK_H

#include <abb/ll/brick.h>
#include <abb/ll/proxyBrick.h>
#include <abb/ll/successor.h>

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <functional>
#include <memory>

namespace abb {

template<typename Done>
class Block {};

template<typename... Args>
class Block<void(Args...)> {
private:
    typedef void ResultType(Args...);
    typedef Block<ResultType> ThisType;
    typedef ll::Brick<ResultType> BrickType;
    typedef ll::Successor<ResultType> SuccessorType;

public:
    Block(std::unique_ptr<BrickType> brick);
    Block(ThisType &&) = default;

    ~Block() {
        if (!this->empty()) {
            this->detach();
        }
    }

    bool empty() const {
        return !this->brick;
    }

    template<typename OutBlock>
    OutBlock pipe(std::function<OutBlock(Args...)> cont);

private:
    void detach();

    std::unique_ptr<BrickType> brick;
};


template<typename... Args>
Block<void(Args...)>::Block(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {}

template<typename... Args>
template<typename OutBlock>
OutBlock Block<void(Args...)>::pipe(std::function<OutBlock(Args...)> cont) {
    typedef typename OutBlock::BrickType OutBrickType;

    typedef ll::ProxyBrick<typename OutBlock::ResultType> ProxyType;

    class LeftSuccessor : public SuccessorType {
    public:
        LeftSuccessor(
            std::unique_ptr<BrickType> brick,
            std::function<OutBlock(Args...)> cont,
            ProxyType & proxy
        ):
            brick(std::move(brick)),
            cont(cont),
            proxy(proxy)
        {
            this->brick->setSuccessor(*this);
        }

        virtual void onsuccess(Args... args) {
            this->proxy.setBrick(std::move(this->cont(args...).brick));
            delete this;
        }

    private:
        std::unique_ptr<BrickType> brick;
        std::function<OutBlock(Args...)> cont;
        ProxyType & proxy;
    };

    ABB_ASSERT(this->brick, "Block is empty");

    ProxyType * pipeBlock = new ProxyType();

    new LeftSuccessor(std::move(this->brick), cont, *pipeBlock);

    return std::unique_ptr<OutBrickType>(pipeBlock);
}

template<typename... Args>
void Block<void(Args...)>::detach() {
    class ImplSuccessor : public SuccessorType {
    public:
        ImplSuccessor(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {
            this->brick->setSuccessor(*this);
        }

        virtual void onsuccess(Args... args) {
            delete this;
        }
    private:
        std::unique_ptr<BrickType> brick;
    };

    new ImplSuccessor(std::move(this->brick));
}

} // namespace abb

#endif // ABB_BLOCK_H
