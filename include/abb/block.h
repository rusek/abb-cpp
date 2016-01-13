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

struct Pass {};

Pass pass;

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

    /*
    template<typename SuccessCont = Nop, typename ErrorCont = Nop>
    typename ContPairTraits<SuccessCont, ResultType, ErrorCont, ReasonType>::BlockType pipe(SuccessCont && onsuccess, ErrorCont && onerror);
    */

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

    template<typename InBlock, typename OutBlock, typename Cont>
    friend class PipeSuccessor;
};

namespace internal {

template<typename Arg, typename... Args>
struct Alt {
    typedef Arg Type;
};

template<typename... Args>
struct Alt<void, Args...> : Alt<Args...> {};

} // namespace internal

template<typename BlockTypeOpt = void, typename... Args>
typename internal::Alt<BlockTypeOpt, Block<Und, void(Args...)>>::Type error(Args... args) {
    typedef typename internal::Alt<BlockTypeOpt, Block<Und, void(Args...)>>::Type BlockType;
    typedef ll::ValueBrick<typename BlockType::ResultType, typename BlockType::ReasonType> ValueBrickType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        brick->setReason(args...);
    } catch(...) {
        delete brick;
        throw;
    }
    return BlockType(std::unique_ptr<typename ValueBrickType::BaseType>(brick));
}

template<typename InBlock, typename OutBlock, typename Cont>
class PipeSuccessor {
    //static_assert(false, "Invalid PipeSuccessor template arguments");
};

template<typename... Args, typename OutBlock, typename ContType>
class PipeSuccessor<Block<void(Args...), Und>, OutBlock, ContType> : public ll::Successor<void(Args...), Und> {
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

template<typename... ResultArgs, typename... ReasonArgs, typename OutBlock, typename Cont>
class PipeSuccessor<Block<void(ResultArgs...), void(ReasonArgs...)>, OutBlock, Cont> : public ll::Successor<void(ResultArgs...), void(ReasonArgs...)> {
private:
    typedef void InResultType(ResultArgs...);
    typedef void InReasonType(ReasonArgs...);
    typedef ll::Brick<InResultType, InReasonType> InBrickType;
    typedef typename ll::ProxyBrick<typename OutBlock::ResultType, typename OutBlock::ReasonType> OutProxyBrickType;
    typedef ContTraits<Cont, InResultType> CT;

public:
    PipeSuccessor(
        std::unique_ptr<InBrickType> brick,
        Cont && cont,
        OutProxyBrickType & proxy
    ):
        brick(std::move(brick)),
        cont(std::forward<Cont>(cont)),
        proxy(proxy)
    {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(ResultArgs... args) {
        this->proxy.setBrick(CT::call(this->cont, args...).takeBrick());
        delete this;
    }

    virtual void onerror(ReasonArgs... args) {
        this->proxy.setBrick(abb::error<OutBlock>(args...).takeBrick());
        delete this;
    }

private:
    std::unique_ptr<InBrickType> brick;
    typename CT::ContType cont;
    OutProxyBrickType & proxy;
};

template<typename Result, typename Reason>
Block<Result, Reason>::Block(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {}

template<typename Result, typename Reason>
template<typename ContType>
auto Block<Result, Reason>::pipe(ContType && cont) -> typename ContTraits<ContType, ResultType>::BlockType {
    typedef ContTraits<ContType, ResultType> CT;
    typedef typename CT::BlockType OutBlock;
    typedef ll::Brick<typename OutBlock::ResultType, typename OutBlock::ReasonType> OutBrickType;
    typedef ll::ProxyBrick<typename OutBlock::ResultType, typename OutBlock::ReasonType> ProxyBrickType;

    ProxyBrickType * pipeBlock = new ProxyBrickType();

    new PipeSuccessor<ThisType, OutBlock, ContType>(this->takeBrick(), std::forward<ContType>(cont), *pipeBlock);

    return std::unique_ptr<OutBrickType>(pipeBlock);
}

template<typename Cont, typename Result, typename Reason, typename... Args>
struct ContReturnTraits<Cont, Block<Result, Reason>, Args...> {
    typedef typename std::decay<Cont>::type ContType;
    typedef Block<Result, Reason> BlockType;

    static BlockType call(ContType & cont, Args... args) {
        return cont(args...);
    }
};

template<typename BlockTypeOpt = void, typename... Args>
typename internal::Alt<BlockTypeOpt, Block<void(Args...)>>::Type success(Args... args) {
    typedef typename internal::Alt<BlockTypeOpt, Block<void(Args...)>>::Type BlockType;
    typedef ll::ValueBrick<typename BlockType::ResultType, typename BlockType::ReasonType> ValueBrickType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        brick->setResult(args...);
    } catch(...) {
        delete brick;
        throw;
    }
    return BlockType(std::unique_ptr<typename ValueBrickType::BaseType>(brick));
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

template<typename... Args>
class DetachSuccessor<Und, void(Args...)> : public ll::Successor<Und, void(Args...)> {
public:
    DetachSuccessor(std::unique_ptr<ll::Brick<Und, void(Args...)>> brick): brick(std::move(brick)) {
        this->brick->setSuccessor(*this);
    }

    virtual void onerror(Args...) {
        delete this;
    }
private:
    std::unique_ptr<ll::Brick<Und, void(Args...)>> brick;
};

template<typename... ResultArgs, typename... ReasonArgs>
class DetachSuccessor<void(ResultArgs...), void(ReasonArgs...)> : public ll::Successor<void(ResultArgs...), void(ReasonArgs...)> {
public:
    DetachSuccessor(std::unique_ptr<ll::Brick<void(ResultArgs...), void(ReasonArgs...)>> brick): brick(std::move(brick)) {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(ResultArgs...) {
        delete this;
    }

    virtual void onerror(ReasonArgs...) {
        delete this;
    }
private:
    std::unique_ptr<ll::Brick<void(ResultArgs...), void(ReasonArgs...)>> brick;
};

template<typename Result, typename Reason>
void Block<Result, Reason>::detach() {
    new DetachSuccessor<ResultType, ReasonType>(this->takeBrick());
}

} // namespace abb

#endif // ABB_BLOCK_H
