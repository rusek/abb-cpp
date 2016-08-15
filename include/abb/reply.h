#ifndef ABB_REPLY_H
#define ABB_REPLY_H

#include <abb/block_fwd.h>
#include <abb/island.h>

#include <abb/utils/debug.h>

namespace abb {

namespace internal {

template<typename Result, typename Reason>
struct reply2 {
    typedef Result result;
    typedef Reason reason;

    virtual ~reply2() {}

    virtual island & get_island() const = 0;

    virtual bool is_aborted() const = 0;
    virtual void set_aborted() = 0;
};

template<typename Result, typename Reason>
struct reply1 : reply2<Result, Reason> {};

template<typename... Args, typename Reason>
struct reply1<void(Args...), Reason> : reply2<void(Args...), Reason> {
    virtual void set_result(Args... args) = 0;
};

template<typename Result, typename Reason>
struct reply : reply1<Result, Reason> {};

template<typename Result, typename... Args>
struct reply<Result, void(Args...)> : reply1<Result, void(Args...)> {
    virtual void set_reason(Args... args) = 0;
};

} // namespace internal

template<typename Result, typename Reason>
struct base_reply2 {
    base_reply2(internal::reply<Result, Reason> * wrapped): wrapped(wrapped) {}

    internal::reply<Result, Reason> * wrapped;
};

template<typename... Args, typename Reason>
struct base_reply2<void(Args...), Reason> {
    base_reply2(internal::reply<void(Args...), Reason> * wrapped): wrapped(wrapped) {}

    void set_result(Args... args) {
        ABB_ASSERT(this->wrapped, "reply is in invalid state");
        this->wrapped->set_result(std::forward<Args>(args)...);
        this->wrapped = nullptr;
    }

protected:
    internal::reply<void(Args...), Reason> * wrapped;
};

template<typename Result, typename Reason>
struct base_reply1 : base_reply2<Result, Reason> {
    base_reply1(internal::reply<Result, Reason> * wrapped): base_reply2<Result, Reason>(wrapped) {}
};

template<typename Result, typename... Args>
struct base_reply1<Result, void(Args...)> : base_reply2<Result, void(Args...)> {
    base_reply1(internal::reply<Result, void(Args...)> * wrapped): base_reply2<Result, void(Args...)>(wrapped) {}

    void set_reason(Args... args) {
        ABB_ASSERT(this->wrapped, "reply is in invalid state");
        this->wrapped->set_reason(std::forward<Args>(args)...);
        this->wrapped = nullptr;
    }
};

template<typename Result, typename Reason>
class base_reply : public base_reply1<Result, Reason> {
public:
    base_reply(): base_reply1<Result, Reason>(nullptr) {}
    base_reply(base_reply<Result, Reason> const&) = delete;
    base_reply(base_reply<Result, Reason> && other): base_reply1<Result, Reason>(other.wrapped) {
        other.wrapped = nullptr;
    }

    explicit base_reply(internal::reply<Result, Reason> & wrapped): base_reply1<Result, Reason>(&wrapped) {}

    base_reply & operator=(base_reply<Result, Reason> const&) = delete;
    base_reply & operator=(base_reply<Result, Reason> && other) {
        this->wrapped = other.wrapped;
        other.wrapped = nullptr;
        return *this;
    }

    bool valid() const {
        return this->wrapped;
    }

    void set_aborted() {
        ABB_ASSERT(this->wrapped, "reply is in invalid state");
        this->wrapped->set_aborted();
        this->wrapped = nullptr;
    }

    island & get_island() const {
        ABB_ASSERT(this->wrapped, "reply is in invalid state");
        return this->wrapped->get_island();
    }

    bool is_aborted() const {
        ABB_ASSERT(this->wrapped, "reply is in invalid state");
        return this->wrapped->is_aborted();
    }
};

template<typename Result = und_t, typename Reason = und_t>
using reply = base_reply<normalize_value_t<Result>, normalize_value_t<Reason>>;

typedef reply<void> void_reply;
typedef reply<> und_reply;

template<typename Arg>
using get_reply_t = base_reply<get_result_t<Arg>, get_reason_t<Arg>>;

} // namespace abb

#endif // ABB_REPLY_H
