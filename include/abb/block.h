#ifndef ABB_BLOCK_H
#define ABB_BLOCK_H

#include <abb/success_fwd.h>
#include <abb/error_fwd.h>
#include <abb/block_fwd.h>
#include <abb/make_fwd.h>
#include <abb/reply.h>

#include <abb/ll/brick.h>
#include <abb/ll/brick_runner.h>
#include <abb/ll/pipe_brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/abort_brick.h>
#include <abb/ll/bridge.h>

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <functional>
#include <type_traits>

namespace abb {

namespace internal {

template<typename Value, typename Cont, typename Return>
struct coerce_cont {
    typedef Cont type;
};

template<typename Value, typename Cont>
struct prepare_regular_cont {};

template<typename... Args, typename Cont>
struct prepare_regular_cont<void(Args...), Cont> {
    typedef ll::unpacker<
        typename coerce_cont<
            void(Args...),
            Cont,
            typename std::result_of<Cont &&(Args...)>::type
        >::type
    > type;
};

template<typename Value, typename Cont>
struct prepare_cont : prepare_regular_cont<Value, Cont> {
};

template<typename Value>
struct prepare_cont<Value, pass_t> {
    typedef pass_t type;
};

template<typename Value>
struct prepare_cont<Value, und_t> {
    typedef und_t type;
};

template<typename Value, typename Result, typename Reason>
struct prepare_cont<Value, base_block<Result, Reason>> {
    class forwarder {
    public:
        explicit forwarder(base_block<Result, Reason> && block):
            block(std::move(block)) {}

        base_block<Result, Reason> operator()() && {
            return std::move(this->block);
        }

    private:
        base_block<Result, Reason> block;
    };

    typedef ll::unpacker<forwarder> type;
};

} // namespace internal

using ll::handle;

template<typename Result, typename Reason>
class base_block {
private:
    typedef ll::brick_ptr<Result, Reason> brick_ptr_type;

    template<typename SuccessCont, typename ErrorCont>
    struct pipe_traits {
        typedef typename internal::prepare_cont<Result, SuccessCont>::type success_cont;
        typedef typename internal::prepare_cont<Reason, ErrorCont>::type error_cont;
        typedef ll::pipe_brick<Result, Reason, success_cont, error_cont> pipe_brick_type;
        typedef get_block_t<pipe_brick_type> out_block_type;
    };

public:
    typedef Result result;
    typedef Reason reason;

    template<typename OtherResult, typename OtherReason>
    base_block(base_block<OtherResult, OtherReason> && other): brick(std::move(other.brick)) {}

    ~base_block() {}

    bool valid() const {
        return this->brick;
    }

    template<typename SuccessCont, typename ErrorCont>
    typename pipe_traits<
        typename std::decay<SuccessCont>::type,
        typename std::decay<ErrorCont>::type
    >::out_block_type pipe(SuccessCont && success_cont, ErrorCont && error_cont) &&;

    template<typename SuccessCont>
    typename pipe_traits<
        typename std::decay<SuccessCont>::type,
        pass_t
    >::out_block_type pipe(SuccessCont && success_cont) && {
        return std::move(*this).pipe(std::forward<SuccessCont>(success_cont), abb::pass);
    }

private:
    explicit base_block(brick_ptr_type && brick): brick(std::move(brick)) {}

    brick_ptr_type take_brick() {
        ABB_ASSERT(this->brick, "block is empty");
        return std::move(this->brick);
    }

    brick_ptr_type brick;

    template<typename FriendResult, typename FriendReason>
    friend class base_block;

    template<typename FriendResult, typename FriendReason>
    friend base_block<FriendResult, FriendReason> ll::pack_brick_ptr(
        ll::brick_ptr<FriendResult, FriendReason> && brick
    );

    template<typename FriendResult, typename FriendReason>
    friend ll::brick_ptr<FriendResult, FriendReason> ll::unpack_brick_ptr(
        base_block<FriendResult, FriendReason> && block
    );
};

namespace internal {

template<typename... Args, typename Cont>
struct coerce_cont<void(Args...), Cont, void> {
    class type {
    public:
        template<typename Arg>
        explicit type(Arg && cont): cont(std::forward<Arg>(cont)) {}

        base_block<void(), und_t> operator()(Args &&... args) {
            cont(std::forward<Args>(args)...);
            return success();
        }

    private:
        Cont cont;
    };
};

} // namespace internal

template<typename Result, typename Reason>
template<typename SuccessCont, typename ErrorCont>
auto base_block<Result, Reason>::pipe(
    SuccessCont && success_cont,
    ErrorCont && error_cont
) && -> typename pipe_traits<
    typename std::decay<SuccessCont>::type,
    typename std::decay<ErrorCont>::type
>::out_block_type {
    typedef pipe_traits<
        typename std::decay<SuccessCont>::type,
        typename std::decay<ErrorCont>::type
    > traits;

    return typename traits::out_block_type(ll::make_brick<typename traits::pipe_brick_type>(
        this->take_brick(),
        std::forward<SuccessCont>(success_cont),
        std::forward<ErrorCont>(error_cont)
    ));
}

handle enqueue(island & target, base_block<void(), und_t> && block);

void enqueue_external(island & target, base_block<void(), und_t> && block);

} // namespace abb

#endif // ABB_BLOCK_H
