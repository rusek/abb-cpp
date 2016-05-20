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

template<typename ValueT, typename ContT, typename ReturnT>
struct CoerceContImpl {
    typedef ContT Type;
};

template<typename ValueT, typename ContT>
struct PrepareRegularContImpl {};

template<typename... ArgsT, typename ContT>
struct PrepareRegularContImpl<void(ArgsT...), ContT> {
    typedef typename CoerceContImpl<
        void(ArgsT...),
        ContT,
        typename std::result_of<ContT(ArgsT...)>::type
    >::Type CoercedContType;

    typedef typename std::result_of<CoercedContType(ArgsT...)>::type BlockType;
    typedef typename BlockType::template Unpacker<CoercedContType> Type;
};

template<typename ValueT, typename ContT>
struct PrepareContImpl : PrepareRegularContImpl<ValueT, ContT> {
};

template<typename ValueT>
struct PrepareContImpl<ValueT, Pass> {
    typedef Pass Type;
};

template<typename ValueT>
struct PrepareContImpl<ValueT, Und> {
    typedef Und Type;
};

} // namespace internal

using ll::Handle;

template<typename ResultT, typename ReasonT>
class BaseBlock {
private:
    typedef ll::BrickPtr<ResultT, ReasonT> BrickPtrType;

    template<typename SuccessContT, typename ErrorContT>
    struct PipeTraits {
        typedef typename internal::PrepareContImpl<ResultT, SuccessContT>::Type SuccessContType;
        typedef typename internal::PrepareContImpl<ReasonT, ErrorContT>::Type ErrorContType;
        struct AbortContType {
            ll::BrickPtr<Und, Und> operator()() {
                return ll::makeBrick<ll::AbortBrick>();
            }
        };
        typedef ll::PipeBrick<ResultT, ReasonT, SuccessContType, ErrorContType, AbortContType> PipeBrickType;
        typedef GetBlock<PipeBrickType> OutBlockType;
    };

public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    explicit BaseBlock(BrickPtrType brick);

    template<typename OtherResultT, typename OtherReasonT>
    BaseBlock(BaseBlock<OtherResultT, OtherReasonT> && other): brick(std::move(other.brick)) {}

    ~BaseBlock() {}

    bool empty() const {
        return !this->brick;
    }

    template<typename SuccessContT, typename ErrorContT>
    typename PipeTraits<
        typename std::decay<SuccessContT>::type,
        typename std::decay<ErrorContT>::type
    >::OutBlockType pipe(SuccessContT && successCont, ErrorContT && errorCont);

    template<typename SuccessContT>
    typename PipeTraits<
        typename std::decay<SuccessContT>::type,
        Pass
    >::OutBlockType pipe(SuccessContT && successCont) {
        return this->pipe(std::forward<SuccessContT>(successCont), abb::pass);
    }

    Handle enqueue(Island & island = Island::current());

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

    template<typename FriendValueT, typename FriendContT>
    friend struct internal::PrepareRegularContImpl;
};

template<typename ResultT, typename ReasonT>
BaseBlock<ResultT, ReasonT>::BaseBlock(BrickPtrType brick): brick(std::move(brick)) {}

namespace internal {

template<typename... ArgsT, typename ContT>
struct CoerceContImpl<void(ArgsT...), ContT, void> {
    class Type {
    public:
        template<typename ArgT>
        explicit Type(ArgT && cont): cont(std::forward<ArgT>(cont)) {}

        BaseBlock<void(), Und> operator()(ArgsT &&... args) {
            cont(std::forward<ArgsT>(args)...);
            return success();
        }

    private:
        ContT cont;
    };
};

} // namespace internal

template<typename ResultT, typename ReasonT>
template<typename SuccessContT, typename ErrorContT>
auto BaseBlock<ResultT, ReasonT>::pipe(
    SuccessContT && successCont,
    ErrorContT && errorCont
) -> typename PipeTraits<
    typename std::decay<SuccessContT>::type,
    typename std::decay<ErrorContT>::type
>::OutBlockType {
    typedef PipeTraits<
        typename std::decay<SuccessContT>::type,
        typename std::decay<ErrorContT>::type
    > Traits;

    return typename Traits::OutBlockType(ll::makeBrick<typename Traits::PipeBrickType>(
        this->takeBrick(),
        std::forward<SuccessContT>(successCont),
        std::forward<ErrorContT>(errorCont),
        typename Traits::AbortContType()
    ));
}

template<typename ResultT, typename ReasonT>
Handle BaseBlock<ResultT, ReasonT>::enqueue(Island & island) {
    return ll::enqueue(island, this->takeBrick());
}

template<typename ResultT, typename ReasonT>
void BaseBlock<ResultT, ReasonT>::enqueueExternal(Island & island) {
    ll::enqueueExternal(island, this->takeBrick());
}

inline Handle enqueue(Island & island, BaseBlock<void(), void()> block) {
    return block.enqueue(island);
}

inline void enqueueExternal(Island & island, BaseBlock<void(), void()> block) {
    block.enqueueExternal(island);
}

} // namespace abb

#endif // ABB_BLOCK_H
