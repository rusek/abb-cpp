#ifndef ABB_BLOCK_H
#define ABB_BLOCK_H

#include <abb/successFwd.h>
#include <abb/errorFwd.h>
#include <abb/blockFwd.h>

#include <abb/ll/brick.h>
#include <abb/ll/proxyBrick.h>
#include <abb/ll/pureExitBrick.h>
#include <abb/ll/successor.h>
#include <abb/ll/detachSuccessor.h>

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>
#include <abb/utils/callResult.h>
#include <abb/utils/alternative.h>

#include <functional>
#include <memory>
#include <type_traits>

namespace abb {

template<typename ContT, typename ReturnT, typename... ArgsT>
struct ContReturnTraits {};

template<typename ContT, typename ValueT>
struct ContTraits {};

template<typename ContT, typename... ArgsT>
struct ContTraits<ContT, void(ArgsT...)> : ContReturnTraits<ContT, typename utils::CallResult<ContT, ArgsT...>::Type, ArgsT...> {};

struct Pass {};

Pass pass;

template<typename BlockT, typename SuccessContT, typename ErrorContT>
struct BlockContTraits {};

template<typename BlockT, typename SuccessContT>
struct BlockContTraits<BlockT, SuccessContT, Pass> {

};

template<typename ResultT, typename ReasonT>
class Block {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;
    typedef Block<ResultType, ReasonType> BlockType;

private:
    typedef ll::Brick<ResultType, ReasonType> BrickType;
    typedef ll::Successor<ResultType, ReasonType> SuccessorType;

    template<typename SuccessContT, typename ErrorContT>
    struct CT : BlockContTraits<BlockType, std::decay<SuccessContT>, std::decay<ErrorContT>> {};

public:
    explicit Block(std::unique_ptr<BrickType> brick);
    Block(BlockType &&) = default;

    ~Block() {
        if (!this->empty()) {
            this->detach();
        }
    }

    bool empty() const {
        return !this->brick;
    }

    /*
    template<typename SuccessContT = Nop, typename ErrorContT = Nop>
    typename ContPairTraits<SuccessContT, ResultType, ErrorContT, ReasonType>::BlockType pipe(SuccessContT && onsuccess, ErrorContT && onerror);
    */

    template<typename ContT>
    typename ContTraits<ContT, ResultType>::BlockType pipe(ContT && cont);

    BlockType exit(std::function<void()> func);

private:
    void detach();

    std::unique_ptr<BrickType> takeBrick() {
        ABB_ASSERT(this->brick, "Block is empty");
        return std::move(this->brick);
    }

    std::unique_ptr<BrickType> brick;

    template<typename InBlockT, typename OutBlockT, typename ContT>
    friend class PipeSuccessor;
};

template<typename InBlockT, typename OutBlockT, typename ContT>
class PipeSuccessor {
    //static_assert(false, "Invalid PipeSuccessor template arguments");
};

template<typename... ArgsT, typename OutBlockT, typename ContT>
class PipeSuccessor<Block<void(ArgsT...), Und>, OutBlockT, ContT> : public ll::Successor<void(ArgsT...), Und> {
private:
    typedef void ResultType(ArgsT...);
    typedef Und ReasonType;
    typedef ll::Brick<ResultType, ReasonType> BrickType;
    typedef ContTraits<ContT, ResultType> CT;
    typedef ll::ProxyBrick<typename CT::BlockType::ResultType, Und> ProxyType;

public:
    PipeSuccessor(
        std::unique_ptr<BrickType> brick,
        ContT && cont,
        ProxyType & proxy
    ):
        brick(std::move(brick)),
        cont(std::forward<ContT>(cont)),
        proxy(proxy)
    {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(ArgsT... args) {
        this->proxy.setBrick(CT::call(this->cont, args...).takeBrick());
        delete this;
    }

private:
    std::unique_ptr<BrickType> brick;
    typename CT::ContType cont;
    ProxyType & proxy;
};

template<typename... ResultArgsT, typename... ReasonArgsT, typename OutBlockT, typename ContT>
class PipeSuccessor<Block<void(ResultArgsT...), void(ReasonArgsT...)>, OutBlockT, ContT> : public ll::Successor<void(ResultArgsT...), void(ReasonArgsT...)> {
private:
    typedef void InResultType(ResultArgsT...);
    typedef void InReasonType(ReasonArgsT...);
    typedef ll::Brick<InResultType, InReasonType> InBrickType;
    typedef typename ll::ProxyBrick<typename OutBlockT::ResultType, typename OutBlockT::ReasonType> OutProxyBrickType;
    typedef ContTraits<ContT, InResultType> CT;

public:
    PipeSuccessor(
        std::unique_ptr<InBrickType> brick,
        ContT && cont,
        OutProxyBrickType & proxy
    ):
        brick(std::move(brick)),
        cont(std::forward<ContT>(cont)),
        proxy(proxy)
    {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(ResultArgsT... args) {
        this->proxy.setBrick(CT::call(this->cont, args...).takeBrick());
        delete this;
    }

    virtual void onerror(ReasonArgsT... args) {
        this->proxy.setBrick(abb::error<OutBlockT>(args...).takeBrick());
        delete this;
    }

private:
    std::unique_ptr<InBrickType> brick;
    typename CT::ContType cont;
    OutProxyBrickType & proxy;
};

template<typename ResultT, typename ReasonT>
Block<ResultT, ReasonT>::Block(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {}

template<typename ResultT, typename ReasonT>
template<typename ContT>
auto Block<ResultT, ReasonT>::pipe(ContT && cont) -> typename ContTraits<ContT, ResultType>::BlockType {
    typedef ContTraits<ContT, ResultType> CT;
    typedef typename CT::BlockType OutBlockT;
    typedef ll::Brick<typename OutBlockT::ResultType, typename OutBlockT::ReasonType> OutBrickType;
    typedef ll::ProxyBrick<typename OutBlockT::ResultType, typename OutBlockT::ReasonType> ProxyBrickType;

    ProxyBrickType * pipeBlock = new ProxyBrickType();

    new PipeSuccessor<BlockType, OutBlockT, ContT>(this->takeBrick(), std::forward<ContT>(cont), *pipeBlock);

    return OutBlockT(std::unique_ptr<OutBrickType>(pipeBlock));
}

template<typename ContT, typename ResultT, typename ReasonT, typename... ArgsT>
struct ContReturnTraits<ContT, Block<ResultT, ReasonT>, ArgsT...> {
    typedef typename std::decay<ContT>::type ContType;
    typedef Block<ResultT, ReasonT> BlockType;

    static BlockType call(ContType & cont, ArgsT... args) {
        return cont(args...);
    }
};

template<typename ContT, typename... ArgsT>
struct ContReturnTraits<ContT, void, ArgsT...> {
    typedef typename std::decay<ContT>::type ContType;
    typedef Block<void()> BlockType;

    static BlockType call(ContType & cont, ArgsT... args) {
        cont(args...);
        return success();
    }
};

template<typename ResultT, typename ReasonT>
auto Block<ResultT, ReasonT>::exit(std::function<void()> func) -> BlockType {
    return BlockType(std::unique_ptr<BrickType>(new ll::PureExitBrick<ResultType, Und>(func, this->takeBrick())));
}

template<typename ResultT, typename ReasonT>
void Block<ResultT, ReasonT>::detach() {
    new ll::DetachSuccessor<ResultType, ReasonType>(this->takeBrick());
}

} // namespace abb

#endif // ABB_BLOCK_H
