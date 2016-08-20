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

template<>
struct value_bank<und_t, und_t> {
    struct type {
        void destroy(bool) {}
    };
};

template<typename Result, typename Reason>
using value_to_bank_t = typename value_bank<Result, Reason>::type;

template<typename Return>
class abort_notifier : utils::noncopyable {
public:
    static_assert(
        std::is_same<typename std::result_of<Return()>::type, void>::value,
        "Invalid type of abort notification"
    );

    abort_notifier(): initialized(false) {}

    ~abort_notifier() {
        if (this->initialized) {
            this->val.destroy();
        }
    }

    template<typename Func, typename Reply>
    void setup(Func & func, Reply && reply) {
        this->val.init(func(std::move(reply)));
        this->initialized = true;
    }

    void abort() {
        ABB_ASSERT(this->initialized, "not initialized");
        (*this->val)();
    }

private:
    utils::bank<Return> val;
    bool initialized;
};

template<>
class abort_notifier<void> {
public:
    template<typename Func, typename Reply>
    void setup(Func & func, Reply && reply) {
        func(std::move(reply));
    }

    void abort() {}
};

template<typename Result, typename Reason, typename Func>
class impl_brick : public brick<Result, Reason>, protected task, protected abb::internal::reply<Result, Reason> {
public:
    typedef Result result;
    typedef Reason reason;

    template<typename Arg>
    impl_brick(Arg && arg): func(std::forward<Arg>(arg)), cur_status(status::startable), succ(nullptr) {}

    ~impl_brick() {
        if (this->cur_status == status::success) {
            this->value.destroy(true);
        } else if (this->cur_status == status::error) {
            this->value.destroy(false);
        }
    }

    void abort() {
        if (this->cur_status == status::running) {
            this->cur_status = status::startable;
            this->notifier.abort();
            if (this->cur_status == status::startable) {
                this->cur_status = status::running;
            } else {
                this->succ = nullptr;
            }
        }
    }

    void start(successor & succ) {
        ABB_ASSERT(!this->succ, "Already got succ");
        this->succ = &succ;
        this->notifier.setup(this->func, reply<Result, Reason>(*static_cast<abb::internal::reply<Result, Reason>*>(this)));
        if (this->cur_status == status::startable) {
            this->cur_status = status::running;
        } else {
            this->succ = nullptr;
        }
    }

    void adopt(successor & succ) {
        this->succ = &succ;
    }

    status get_status() const {
        if (this->succ) {
            return status::running;
        } else {
            return this->cur_status;
        }
    }

    store<Result> & get_result() {
        ABB_ASSERT(this->cur_status == status::success, "Result is not set");
        return *this->value.result;
    }

    store<Reason> & get_reason() {
        ABB_ASSERT(this->cur_status == status::error, "Reason is not set");
        return *this->value.reason;
    }

protected:
    virtual void run();

    virtual island & get_island() const;
    virtual bool is_aborted() const;
    virtual void set_aborted();

    Func func;
    abort_notifier<typename std::result_of<Func(reply<Result, Reason>)>::type> notifier;
    value_to_bank_t<result, reason> value;
    status cur_status;
    successor * succ;
};

template<typename Result, typename Reason, typename Func>
void impl_brick<Result, Reason, Func>::run() {
    successor * succ = this->succ;
    this->succ = nullptr;
    succ->on_update();
}

template<typename Result, typename Reason, typename Func>
island & impl_brick<Result, Reason, Func>::get_island() const {
    ABB_ASSERT(
        this->cur_status == status::running || this->cur_status == status::startable,
        "Cannot call get_island after setting a value"
    );
    return this->succ->get_island();
}

template<typename Result, typename Reason, typename Func>
bool impl_brick<Result, Reason, Func>::is_aborted() const {
    ABB_ASSERT(
        this->cur_status == status::running || this->cur_status == status::startable,
        "Cannot call get_island after setting a value"
    );
    return this->succ->is_aborted();
}

template<typename Result, typename Reason, typename Func>
void impl_brick<Result, Reason, Func>::set_aborted() {
    ABB_ASSERT(
        this->cur_status == status::running || this->cur_status == status::startable,
        "Already got value"
    );
    ABB_ASSERT(this->succ->is_aborted(), "Abort was not requested");
    status prev_status = this->cur_status;
    this->cur_status = status::abort;
    if (prev_status == status::running) {
        this->succ->get_island().enqueue(static_cast<task&>(*this));
    }
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
        ABB_ASSERT(
            this->cur_status == status::running || this->cur_status == status::startable,
            "Already got value"
        );
        this->value.result.init(box_arg, std::forward<Args>(args)...);
        status prev_status = this->cur_status;
        this->cur_status = status::success;
        if (prev_status == status::running) {
            this->succ->get_island().enqueue(static_cast<task&>(*this));
        }
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
        ABB_ASSERT(
            this->cur_status == status::running || this->cur_status == status::startable,
            "Already got value"
        );
        this->value.reason.init(box_arg, std::forward<Args>(args)...);
        status prev_status = this->cur_status;
        this->cur_status = status::error;
        if (prev_status == status::running) {
            this->succ->get_island().enqueue(static_cast<task&>(*this));
        }
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
