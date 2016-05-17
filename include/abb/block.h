#ifndef ABB_BLOCK_H
#define ABB_BLOCK_H

#include <abb/successFwd.h>
#include <abb/errorFwd.h>
#include <abb/blockFwd.h>
#include <abb/makeFwd.h>
#include <abb/reply.h>

#include <abb/ll/brick.h>
#include <abb/ll/runner.h>
#include <abb/ll/pipeBrick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/abortBrick.h>

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <functional>
#include <type_traits>

namespace abb {

namespace internal {

template<typename ContT, typename ReturnT, typename... ArgsT>
struct ContTraitsWithReturn {
    typedef ReturnT BlockType;
    typedef ContT ContType;
};

template<typename ContT, typename... ArgsT>
struct ContTraits : ContTraitsWithReturn<ContT, typename std::result_of<ContT(ArgsT...)>::type, ArgsT...> {};

template<typename BlockT, typename SuccessContT, typename ErrorContT>
struct BlockContTraits {};

template<typename... ResultArgsT, typename SuccessContT>
class BlockContTraits<BaseBlock<void(ResultArgsT...), Und>, SuccessContT, Pass> {
private:
    typedef ContTraits<SuccessContT, ResultArgsT...> SuccessContTraits;
public:
    typedef typename SuccessContTraits::BlockType BlockType;
    typedef typename SuccessContTraits::ContType SuccessContType;
    typedef Und ErrorContType;
};

template<typename... ReasonArgsT, typename ErrorContT>
class BlockContTraits<BaseBlock<Und, void(ReasonArgsT...)>, Pass, ErrorContT> {
private:
    typedef ContTraits<ErrorContT, ReasonArgsT...> ErrorContTraits;
public:
    typedef typename ErrorContTraits::BlockType BlockType;
    typedef Und SuccessContType;
    typedef typename ErrorContTraits::ContType ErrorContType;
};

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT>
class BlockContTraits<BaseBlock<void(ResultArgsT...), void(ReasonArgsT...)>, SuccessContT, Pass> {
private:
    typedef ContTraits<SuccessContT, ResultArgsT...> SuccessContTraits;
public:
    typedef typename SuccessContTraits::BlockType BlockType;
    typedef typename SuccessContTraits::ContType SuccessContType;
    struct ErrorContType {
        BlockType operator()(ReasonArgsT &&... args) {
            return error<BlockType>(std::forward<ReasonArgsT>(args)...);
        }
    };
};

template<typename... ResultArgsT, typename... ReasonArgsT, typename ErrorContT>
class BlockContTraits<BaseBlock<void(ResultArgsT...), void(ReasonArgsT...)>, Pass, ErrorContT> {
private:
    typedef ContTraits<ErrorContT, ReasonArgsT...> ErrorContTraits;
public:
    typedef typename ErrorContTraits::BlockType BlockType;
    struct SuccessContType {
        BlockType operator()(ResultArgsT &&... args) {
            return success<BlockType>(std::forward<ResultArgsT>(args)...);
        }
    };
    typedef typename ErrorContTraits::ContType ErrorContType;
};

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
class BlockContTraits<BaseBlock<void(ResultArgsT...), void(ReasonArgsT...)>, SuccessContT, ErrorContT> {
private:
    typedef ContTraits<SuccessContT, ResultArgsT...> SuccessContTraits;
    typedef ContTraits<ErrorContT, ReasonArgsT...> ErrorContTraits;
public:
    typedef typename SuccessContTraits::BlockType BlockType;
    static_assert(
        std::is_same<BlockType, typename ErrorContTraits::BlockType>::value,
        "Blocks returned by continuations are of different types"
    );

    typedef typename SuccessContTraits::ContType SuccessContType;
    typedef typename ErrorContTraits::ContType ErrorContType;
};

} // namespace internal


template<typename ResultT, typename ReasonT>
class BaseBlock {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;
    typedef BaseBlock<ResultType, ReasonType> BlockType;
    typedef BaseReply<ResultType, ReasonType> ReplyType;

private:
    typedef ll::BrickPtr<ResultType, ReasonType> BrickPtrType;

public:
    explicit BaseBlock(BrickPtrType brick);

    template<typename OtherResultT, typename OtherReasonT>
    BaseBlock(BaseBlock<OtherResultT, OtherReasonT> && other): brick(std::move(other.brick)) {}

    ~BaseBlock() {}

    bool empty() const {
        return !this->brick;
    }

    template<typename SuccessContT, typename ErrorContT>
    typename internal::BlockContTraits<
        BlockType,
        typename std::decay<SuccessContT>::type,
        typename std::decay<ErrorContT>::type
    >::BlockType pipe(SuccessContT && successCont, ErrorContT && errorCont);

    template<typename SuccessContT>
    typename internal::BlockContTraits<
        BlockType,
        typename std::decay<SuccessContT>::type,
        Pass
    >::BlockType pipe(SuccessContT && successCont) {
        return this->pipe(std::forward<SuccessContT>(successCont), abb::pass);
    }

    void enqueue(Island & island = Island::current());

    void enqueueExternal(Island & island);

private:
    template<typename ContT>
    class Unpacker {
    public:
        template<typename... ArgsT>
        explicit Unpacker(ArgsT &&... args): cont(std::forward<ArgsT>(args)...) {}

        template<typename... ArgsT>
        BrickPtrType operator()(ArgsT &&... args) {
            return cont(std::forward<ArgsT>(args)...).takeBrick();
        }

    private:
        ContT cont;
    };

    BrickPtrType takeBrick() {
        ABB_ASSERT(this->brick, "Block is empty");
        return std::move(this->brick);
    }

    BrickPtrType brick;

    template<typename FriendResultT, typename FriendReasonT>
    friend class BaseBlock;

    template<typename FriendFuncT, typename... FriendArgsT>
    friend internal::MakeReturn<FriendFuncT> make(FriendArgsT &&... args);
};

template<typename ResultT, typename ReasonT>
BaseBlock<ResultT, ReasonT>::BaseBlock(BrickPtrType brick): brick(std::move(brick)) {}

namespace internal {

template<typename ContT, typename... ArgsT>
struct ContTraitsWithReturn<ContT, void, ArgsT...> {
    typedef BaseBlock<void(), Und> BlockType;
    class ContType {
    public:
        template<typename ArgT>
        explicit ContType(ArgT && cont): cont(std::forward<ArgT>(cont)) {}

        BlockType operator()(ArgsT &&... args) {
            cont(std::forward<ArgsT>(args)...);
            return success<BlockType>();
        }

    private:
        ContT cont;
    };
};

template<typename ReturnT, typename DecayedArgT, typename ArgT>
struct ContBuilder {
    static ReturnT build(ArgT && arg) {
        return ReturnT(std::forward<ArgT>(arg));
    }
};

template<typename ReturnT, typename ArgT>
struct ContBuilder<ReturnT, Pass, ArgT> {
    static ReturnT build(ArgT &&) {
        return ReturnT();
    }
};


} // namespace internal

template<typename ResultT, typename ReasonT>
template<typename SuccessContT, typename ErrorContT>
auto BaseBlock<ResultT, ReasonT>::pipe(
    SuccessContT && successCont,
    ErrorContT && errorCont
) -> typename internal::BlockContTraits<
    BlockType,
    typename std::decay<SuccessContT>::type,
    typename std::decay<ErrorContT>::type
>::BlockType {
    typedef typename std::decay<SuccessContT>::type SuccessContD;
    typedef typename std::decay<ErrorContT>::type ErrorContD;
    typedef internal::BlockContTraits<BlockType, SuccessContD, ErrorContD> Traits;
    typedef typename Traits::BlockType OutBlockType;
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
    typedef internal::ContBuilder<BrickSuccessContType, SuccessContD, SuccessContT> SuccessContBuilder;
    typedef internal::ContBuilder<BrickErrorContType, ErrorContD, ErrorContT> ErrorContBuilder;

    struct AbortCont {
        ll::BrickPtr<Und, Und> operator()() {
            return ll::makeBrick<ll::AbortBrick>();
        }
    };

    return OutBlockType(ll::makeBrick<ll::PipeBrick<
        ResultType,
        ReasonType,
        BrickSuccessContType,
        BrickErrorContType,
        AbortCont
    >>(
        this->takeBrick(),
        SuccessContBuilder::build(std::forward<SuccessContT>(successCont)),
        ErrorContBuilder::build(std::forward<ErrorContT>(errorCont)),
        AbortCont()
    ));
}

template<typename ResultT, typename ReasonT>
void BaseBlock<ResultT, ReasonT>::enqueue(Island & island) {
    ll::enqueue(island, this->takeBrick());
}

template<typename ResultT, typename ReasonT>
void BaseBlock<ResultT, ReasonT>::enqueueExternal(Island & island) {
    ll::enqueue(island, this->takeBrick());
}

inline void enqueue(Island & island, BaseBlock<void(), void()> block) {
    block.enqueue(island);
}

inline void enqueueExternal(Island & island, BaseBlock<void(), void()> block) {
    block.enqueueExternal(island);
}

} // namespace abb

#endif // ABB_BLOCK_H
