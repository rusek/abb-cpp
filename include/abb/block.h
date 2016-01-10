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

template<typename Cont, typename Value>
struct ContTraits {};

template<typename Cont, typename... Args>
struct ContTraits<Cont, void(Args...)> : ContReturnTraits<Cont, typename utils::CallResult<Cont, Args...>::Type, Args...> {};

template<typename Result = Und, typename Reason = Und>
class Block {
public:
    typedef Result ResultType;
    typedef Reason ReasonType;

private:
    typedef Block<ResultType, ReasonType> ThisType;
    typedef ll::Brick<ResultType, ReasonType> BrickType;
    typedef ll::Successor<ResultType, ReasonType> SuccessorType;

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
    typename ContTraits<ContType, ResultType>::BlockType pipe(ContType && cont);

    ThisType exit(std::function<void()> func);

private:
    void detach();

    std::unique_ptr<BrickType> takeBrick() {
        ABB_ASSERT(this->brick, "Block is empty");
        return std::move(this->brick);
    }

    std::unique_ptr<BrickType> brick;

    template<typename BlockType, typename ContType>
    friend class PipeSuccessor;
};


template<typename BlockType, typename ContType>
class PipeSuccessor {};

template<typename... Args, typename ContType>
class PipeSuccessor<Block<void(Args...), Und>, ContType> : public ll::Successor<void(Args...), Und> {
private:
    typedef void ResultType(Args...);
    typedef Und ReasonType;
    typedef ll::Brick<ResultType, ReasonType> BrickType;
    typedef ContTraits<ContType, ResultType> CT;
    typedef ll::ProxyBrick<typename CT::BlockType::ResultType, Und> ProxyType;

public:
    PipeSuccessor(
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


template<typename Result, typename Reason>
Block<Result, Reason>::Block(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {}

template<typename Result, typename Reason>
template<typename ContType>
auto Block<Result, Reason>::pipe(ContType && cont) -> typename ContTraits<ContType, ResultType>::BlockType {
    typedef ContTraits<ContType, ResultType> CT;
    typedef typename CT::BlockType OutBlock;
    typedef typename OutBlock::BrickType OutBrickType;

    typedef ll::ProxyBrick<typename OutBlock::ResultType, Und> ProxyType;

    ProxyType * pipeBlock = new ProxyType();

    new PipeSuccessor<ThisType, ContType>(this->takeBrick(), std::forward<ContType>(cont), *pipeBlock);

    return std::unique_ptr<OutBrickType>(pipeBlock);
}

template<typename Cont, typename Result, typename... Args>
struct ContReturnTraits<Cont, Block<Result, Und>, Args...> {
    typedef typename std::decay<Cont>::type ContType;
    typedef Block<Result, Und> BlockType;

    static BlockType call(ContType & cont, Args... args) {
        return cont(args...);
    }
};

template<typename... Args>
Block<void(Args...)> success(Args... args) {
    typedef ll::SuccessBrick<void(Args...), Und> BrickType;

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

template<typename Result, typename Reason>
auto Block<Result, Reason>::exit(std::function<void()> func) -> ThisType {
    return std::unique_ptr<BrickType>(new ll::PureExitBrick<ResultType, Und>(func, this->takeBrick()));
}

template<typename Result, typename Reason>
class DetachSuccessor {};

template<typename... Args>
class DetachSuccessor<void(Args...), Und> : public ll::Successor<void(Args...), Und> {
public:
    DetachSuccessor(std::unique_ptr<ll::Brick<void(Args...), Und>> brick): brick(std::move(brick)) {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(Args...) {
        delete this;
    }
private:
    std::unique_ptr<ll::Brick<void(Args...), Und>> brick;
};


template<typename Result, typename Reason>
void Block<Result, Reason>::detach() {
    new DetachSuccessor<ResultType, ReasonType>(this->takeBrick());
}

} // namespace abb

#endif // ABB_BLOCK_H
