#ifndef ABB_LL_IMPL_BRICK_H
#define ABB_LL_IMPL_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>
#include <abb/reply.h>

#include <abb/utils/debug.h>
#include <abb/utils/bank.h>

namespace abb {
namespace ll {

namespace internal {

template<typename Result, typename Reason>
struct value_bank {
    union type {
        void destroy(bool success) {
            if (success) {
                this->result.destroy();
            } else {
                this->reason.destroy();
            }
        }

        utils::bank<store<Result>> result;
        utils::bank<store<Reason>> reason;
    };
};

template<typename Result>
struct value_bank<Result, und_t> {
    struct type {
        void destroy(bool) {
            this->result.destroy();
        }

        utils::bank<store<Result>> result;
    };
};

template<typename Reason>
struct value_bank<und_t, Reason> {
    struct type {
        void destroy(bool) {
            this->reason.destroy();
        }

        utils::bank<store<Reason>> reason;
    };
};

template<typename Result, typename Reason>
using value_to_bank_t = typename value_bank<Result, Reason>::type;

template<typename Result, typename Reason, typename Func>
class impl_brick : public brick<Result, Reason>, protected task, protected reply<Result, Reason> {
public:
    typedef Result result;
    typedef Reason reason;

    template<typename Arg>
    impl_brick(Arg && arg): func(std::forward<Arg>(arg)), cur_status(pending_status), succ(nullptr) {}

    ~impl_brick() {
        if (this->cur_status & (success_status | error_status)) {
            this->value.destroy(this->cur_status & success_status);
        }
    }

    void abort() {}

    void start(successor & succ) {
        ABB_ASSERT(!this->succ, "Already got succ");
        this->succ = &succ;
        this->func(*static_cast<reply<Result, Reason>*>(this));
    }

    status get_status() const {
        return this->cur_status;
    }

    store<Result> & get_result() {
        ABB_ASSERT(this->cur_status & success_status, "Result is not set");
        return *this->value.result;
    }

    store<Reason> & get_reason() {
        ABB_ASSERT(this->cur_status & error_status, "Reason is not set");
        return *this->value.reason;
    }

protected:
    virtual void run();

    virtual island & get_island() const;
    virtual bool is_aborted() const;
    virtual void set_aborted();

    Func func;
    internal::value_to_bank_t<result, reason> value;
    status cur_status;
    successor * succ;
};

template<typename Result, typename Reason, typename Func>
void impl_brick<Result, Reason, Func>::run() {
    this->succ->on_update();
}

template<typename Result, typename Reason, typename Func>
island & impl_brick<Result, Reason, Func>::get_island() const {
    ABB_ASSERT(this->cur_status == pending_status, "Cannot call get_island after setting a value");
    return this->succ->get_island();
}

template<typename Result, typename Reason, typename Func>
bool impl_brick<Result, Reason, Func>::is_aborted() const {
    ABB_ASSERT(this->cur_status == pending_status, "Cannot call get_island after setting a value");
    return this->succ->is_aborted();
}

template<typename Result, typename Reason, typename Func>
void impl_brick<Result, Reason, Func>::set_aborted() {
    ABB_ASSERT(this->cur_status == pending_status, "Already got value");
    ABB_ASSERT(this->succ->is_aborted(), "Abort was not requested");
    this->cur_status = abort_status;
    this->succ->get_island().enqueue(static_cast<task&>(*this));
}

template<typename Result, typename Reason, typename Func>
class impl_brick_success : public impl_brick<Result, Reason, Func> {
public:
    using impl_brick<Result, Reason, Func>::impl_brick;
};

template<typename... Args, typename Reason, typename Func>
class impl_brick_success<void(Args...), Reason, Func> : public impl_brick<void(Args...), Reason, Func> {
public:
    using impl_brick<void(Args...), Reason, Func>::impl_brick;

protected:
    virtual void set_result(Args... args) {
        ABB_ASSERT(this->cur_status == pending_status, "Already got value");
        this->value.result.init(std::forward<Args>(args)...);
        this->cur_status = success_status;
        this->succ->get_island().enqueue(static_cast<task&>(*this));
    }
};

template<typename Result, typename Reason, typename Func>
class impl_brick_error : public impl_brick_success<Result, Reason, Func> {
public:
    using impl_brick_success<Result, Reason, Func>::impl_brick_success;
};

template<typename Result, typename... Args, typename Func>
class impl_brick_error<Result, void(Args...), Func> : public impl_brick_success<Result, void(Args...), Func> {
public:
    using impl_brick_success<Result, void(Args...), Func>::impl_brick_success;

protected:
    virtual void set_reason(Args... args) {
        ABB_ASSERT(this->cur_status == pending_status, "Already got value");
        this->value.reason.init(std::forward<Args>(args)...);
        this->cur_status = error_status;
        this->succ->get_island().enqueue(static_cast<task&>(*this));
    }
};

} // namespace internal

template<typename Result, typename Reason, typename Func>
class impl_brick : public internal::impl_brick_error<Result, Reason, Func> {
public:
    using internal::impl_brick_error<Result, Reason, Func>::impl_brick_error;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_IMPL_BRICK_H
