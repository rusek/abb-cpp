#ifndef ABB_BLOCK_H
#define ABB_BLOCK_H

#include <abb/ll/brick.h>
#include <abb/ll/proxyBrick.h>
#include <abb/ll/pureExitBrick.h>
#include <abb/ll/successor.h>

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>
#include <abb/utils/callResult.h>

#include <functional>
#include <memory>
#include <type_traits>

namespace abb {

template<typename Cont, typename Return, typename... Args>
struct ContReturnTraits {};

template<typename Cont, typename... Args>
struct ContTraits : ContReturnTraits<Cont, typename utils::CallResult<Cont, Args...>::Type, Args...> {};

template<typename Done>
class Block {};

template<typename... Args>
class Block<void(Args...)> {
public:
    typedef void ResultType(Args...);

private:
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

    template<typename ContType>
    typename ContTraits<ContType, Args...>::BlockType pipe(ContType && cont);

    ThisType exit(std::function<void()> func);

private:
    void detach();

    std::unique_ptr<BrickType> takeBrick() {
        ABB_ASSERT(this->brick, "Block is empty");
        return std::move(this->brick);
    }

    std::unique_ptr<BrickType> brick;
};


template<typename... Args>
Block<void(Args...)>::Block(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {}

template<typename... Args>
template<typename ContType>
typename ContTraits<ContType, Args...>::BlockType Block<void(Args...)>::pipe(ContType && cont) {
    typedef ContTraits<ContType, Args...> CT;
    typedef typename CT::BlockType OutBlock;
    typedef typename OutBlock::BrickType OutBrickType;

    typedef ll::ProxyBrick<typename OutBlock::ResultType> ProxyType;

    class LeftSuccessor : public SuccessorType {
    public:
        LeftSuccessor(
            std::unique_ptr<BrickType> brick,
            ContType && cont,
            ProxyType & proxy
        ):
            brick(std::move(brick)),
            cont(std::forward<ContType>(cont)),
            proxy(proxy)
        {
            this->brick->setSuccessor(*this);
        }

        virtual void onsuccess(Args... args) {
            this->proxy.setBrick(CT::call(this->cont, args...).takeBrick());
            delete this;
        }

    private:
        std::unique_ptr<BrickType> brick;
        typename CT::ContType cont;
        ProxyType & proxy;
    };

    ProxyType * pipeBlock = new ProxyType();

    new LeftSuccessor(this->takeBrick(), std::forward<ContType>(cont), *pipeBlock);

    return std::unique_ptr<OutBrickType>(pipeBlock);
}

template<typename Cont, typename Result, typename... Args>
struct ContReturnTraits<Cont, Block<Result>, Args...> {
    typedef typename std::decay<Cont>::type ContType;
    typedef Block<Result> BlockType;

    static BlockType call(ContType & cont, Args... args) {
        return cont(args...);
    }
};

template<typename... Args>
Block<void(Args...)> success(Args... args) {
    typedef ll::SuccessBrick<void(Args...)> BrickType;

    BrickType * brick = new BrickType;
    try {
        brick->setResult(args...);
    } catch(...) {
        delete brick;
        throw;
    }
    return Block<void(Args...)>(std::unique_ptr<typename BrickType::BaseType>(brick));
}

template<typename Cont, typename... Args>
struct ContReturnTraits<Cont, void, Args...> {
    typedef typename std::decay<Cont>::type ContType;
    typedef Block<void()> BlockType;

    static BlockType call(ContType & cont, Args... args) {
        cont(args...);
        return success();
    }
};

template<typename... Args>
auto Block<void(Args...)>::exit(std::function<void()> func) -> ThisType {
    return std::unique_ptr<BrickType>(new ll::PureExitBrick<ResultType>(func, this->takeBrick()));
}

template<typename... Args>
void Block<void(Args...)>::detach() {
    class ImplSuccessor : public SuccessorType {
    public:
        ImplSuccessor(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {
            this->brick->setSuccessor(*this);
        }

        virtual void onsuccess(Args...) {
            delete this;
        }
    private:
        std::unique_ptr<BrickType> brick;
    };

    new ImplSuccessor(std::move(this->brick));
}

} // namespace abb

#endif // ABB_BLOCK_H
