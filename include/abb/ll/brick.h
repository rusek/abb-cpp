#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/ll/store.h>

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

} // namespace internal

class successor {
public:
    virtual ~successor() {}
    virtual void on_update() = 0;
    virtual island & get_island() const = 0;
    virtual bool is_aborted() const = 0;
};

enum {
    pending_status = 0,
    success_status = 1,
    error_status = 2,
    abort_status = 4,
    next_status = 8
};

typedef int status;

template<typename Result, typename Reason>
class brick_ptr;

template<typename Result, typename Reason>
struct brick : private internal::raw_brick {
    typedef Result result;
    typedef Reason reason;

    template<typename FriendBrick>
    friend class internal::brick_funcs;

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
