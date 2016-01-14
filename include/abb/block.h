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
#include <abb/ll/pipeBrick.h>

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

struct Pass {
    operator Und() const {
        return Und();
    }
};

Pass pass;

template<typename BlockT, typename SuccessContT, typename ErrorContT>
struct BlockContTraits {};

template<typename... ResultArgsT, typename SuccessContT>
struct BlockContTraits<Block<void(ResultArgsT...), Und>, SuccessContT, Pass> {
    typedef typename utils::CallResult<SuccessContT, ResultArgsT...>::Type BlockType;
    typedef SuccessContT SuccessContType;
    typedef Und ErrorContType;
};

template<typename... ReasonArgsT, typename ErrorContT>
struct BlockContTraits<Block<Und, void(ReasonArgsT...)>, Pass, ErrorContT> {
    typedef typename utils::CallResult<ErrorContT, ReasonArgsT...>::Type BlockType;
    typedef Und SuccessContType;
    typedef ErrorContT ErrorContType;
};

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
struct BlockContTraits<Block<void(ResultArgsT...), void(ReasonArgsT...)>, SuccessContT, ErrorContT> {
    typedef typename utils::CallResult<SuccessContT, ResultArgsT...>::Type BlockType;
    typedef SuccessContT SuccessContType;
    typedef ErrorContT ErrorContType;
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

    template<typename SuccessContT, typename ErrorContT>
    typename BlockContTraits<
        BlockType,
        typename std::decay<SuccessContT>::type,
        typename std::decay<ErrorContT>::type
    >::BlockType pipe(SuccessContT && successCont, ErrorContT && errorCont);

    template<typename SuccessContT>
    typename BlockContTraits<
        BlockType,
        typename std::decay<SuccessContT>::type,
        Pass
    >::BlockType pipe(SuccessContT && successCont) {
        return this->pipe(std::forward<SuccessContT>(successCont), abb::pass);
    }

    BlockType exit(std::function<void()> func);

private:
    template<typename ContT>
    class Unpacker {
    public:
        explicit Unpacker(ContT && cont): cont(cont) {}

        template<typename... ArgsT>
        std::unique_ptr<BrickType> operator()(ArgsT... args) {
            return cont(args...).takeBrick();
        }

    private:
        ContT cont;
    };

    void detach();

    std::unique_ptr<BrickType> takeBrick() {
        ABB_ASSERT(this->brick, "Block is empty");
        return std::move(this->brick);
    }

    std::unique_ptr<BrickType> brick;
};

template<typename ResultT, typename ReasonT>
Block<ResultT, ReasonT>::Block(std::unique_ptr<BrickType> brick): brick(std::move(brick)) {}

template<typename ResultT, typename ReasonT>
template<typename SuccessContT, typename ErrorContT>
auto Block<ResultT, ReasonT>::pipe(SuccessContT && successCont, ErrorContT && errorCont) -> typename BlockContTraits<
    BlockType,
    typename std::decay<SuccessContT>::type,
    typename std::decay<ErrorContT>::type
>::BlockType {
    typedef BlockContTraits<
        BlockType,
        typename std::decay<SuccessContT>::type,
        typename std::decay<ErrorContT>::type
    > Traits;
    typedef typename Traits::BlockType OutBlockType;
    typedef typename OutBlockType::BrickType OutBrickType;
    typedef typename std::conditional<
        IsDefined<typename Traits::SuccessContType>::value,
        typename OutBlockType::template Unpacker<typename Traits::SuccessContType>,
        Und
    >::type BrickSuccessContType;
    typedef typename std::conditional<
        IsDefined<typename Traits::ErrorContType>::value,
        typename OutBlockType::template Unpacker<typename Traits::ErrorContType>,
        Und
    >::type BrickErrorContType;

    return OutBlockType(std::unique_ptr<OutBrickType>(new ll::PipeBrick<
        ResultType,
        ReasonType,
        BrickSuccessContType,
        BrickErrorContType
    >(
        this->takeBrick(),
        BrickSuccessContType(std::forward<SuccessContT>(successCont)),
        BrickErrorContType(std::forward<ErrorContT>(errorCont))
    )));
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
