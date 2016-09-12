#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/ll/box.h>

#include <abb/island.h>
#include <abb/value.h>

#include <abb/utils/noncopyable.h>

namespace abb {
namespace ll {

namespace internal {

struct brick_vtable;

class raw_brick : private utils::noncopyable {
public:
    internal::brick_vtable const* vtable;
};

template<typename FriendBrick>
class brick_funcs;

template<typename Value>
struct get_store;

template<typename... Args>
struct get_store<void(Args...)> {
    typedef boxes<Args...> type;
};

template<>
struct get_store<und_t> {
    typedef und_t type;
};

} // namespace internal

template<typename Value>
using store = typename internal::get_store<Value>::type;

class successor {
public:
    virtual ~successor() {}
    virtual void on_update() = 0;
};

enum class status {
    running,
    startable,
    next,
    success,
    error,
    abort
};

template<typename Result, typename Reason>
class brick_ptr;

template<typename Result, typename Reason>
struct brick : private internal::raw_brick {
    typedef Result result;
    typedef Reason reason;

    template<typename FriendBrick>
    friend class internal::brick_funcs;

    void start(island &, bool, successor &) {
        ABB_FIASCO("Erased method called");
    }

    void adopt(successor &) {
        ABB_FIASCO("Erased method called");
    }

    void abort() {
        ABB_FIASCO("Erased method called");
    }

    brick_ptr<Result, Reason> get_next() {
        ABB_FIASCO("Erased method called");
    }

    store<Result> & get_result() {
        ABB_FIASCO("Erased method called");
    }

    store<Reason> & get_reason() {
        ABB_FIASCO("Erased method called");
    }
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
