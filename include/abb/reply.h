#ifndef ABB_REPLY_H
#define ABB_REPLY_H

#include <abb/block_fwd.h>
#include <abb/island.h>

namespace abb {

namespace internal {

template<typename Result, typename Reason>
struct reply {
    typedef Result result;
    typedef Reason reason;

    virtual ~reply() {}

    virtual island & get_island() const = 0;

    virtual bool is_aborted() const = 0;
    virtual void set_aborted() = 0;
};

template<typename Result, typename Reason>
struct success_reply : reply<Result, Reason> {};

template<typename... Args, typename Reason>
struct success_reply<void(Args...), Reason> : reply<void(Args...), Reason> {
    virtual void set_result(Args... args) = 0;
};

template<typename Result, typename Reason>
struct error_reply : success_reply<Result, Reason> {};

template<typename Result, typename... Args>
struct error_reply<Result, void(Args...)> : success_reply<Result, void(Args...)> {
    virtual void set_reason(Args... args) = 0;
};

} // namespace internal

template<typename Result, typename Reason>
class base_reply : public internal::error_reply<Result, Reason> {
public:
};

template<typename Result = und_t, typename Reason = und_t>
using reply = base_reply<normalize_value_t<Result>, normalize_value_t<Reason>>;

typedef reply<void> void_reply;
typedef reply<> und_reply;

template<typename Arg>
using get_reply_t = base_reply<get_result_t<Arg>, get_reason_t<Arg>>;

} // namespace abb

#endif // ABB_REPLY_H
