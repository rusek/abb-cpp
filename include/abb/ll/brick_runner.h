#ifndef ABB_LL_BRICK_RUNNER_H
#define ABB_LL_BRICK_RUNNER_H

#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>

#include <abb/value.h>

namespace abb {
namespace ll {

namespace internal {

class handleable {
public:
    virtual void abort() = 0;
    virtual void drop_handle() = 0;
};

enum {
    runner_ref_handle = 1,
    runner_ref_enqueue = 2,
    runner_ref_work = 4,
    runner_ref = runner_ref_handle | runner_ref_enqueue | runner_ref_work,
    runner_abort_requested = 8,
    runner_abort_accepted = 16
};

typedef int runner_flags;

template<typename Result, typename Reason, bool External>
class brick_runner : public handleable, private successor, private task {
public:
    brick_runner(island & target, brick_ptr<Result, Reason> brick):
        target(target),
        brick(std::move(brick)),
        flags(runner_ref_enqueue | runner_ref_handle)
    {
        this->target.incref_external();
        if (External) {
            this->target.enqueue_external(static_cast<task&>(*this));
        } else {
            this->target.enqueue(static_cast<task&>(*this));
        }
    }

    virtual void abort();
    virtual void drop_handle();

private:
    virtual void on_update();
    virtual island & get_island() const;
    virtual bool is_aborted() const;
    virtual void run();

    island & target;
    brick_ptr<Result, Reason> brick;
    runner_flags flags;
};

template<typename Result, typename Reason, bool External>
void brick_runner<Result, Reason, External>::on_update() {
    if (this->brick.try_start(*this)) {
        this->target.decref_external();
        this->flags &= ~runner_ref_work;
        if ((this->flags & runner_ref) == 0) {
            delete this;
        }
    }
}

template<typename Result, typename Reason, bool External>
island & brick_runner<Result, Reason, External>::get_island() const {
    return this->target;
}

template<typename Result, typename Reason, bool External>
bool brick_runner<Result, Reason, External>::is_aborted() const {
    return (this->flags & runner_abort_accepted) != 0;
}

template<typename Result, typename Reason, bool External>
void brick_runner<Result, Reason, External>::run() {
    this->flags &= ~runner_ref_enqueue;
    if (this->flags & runner_abort_requested) {
        this->flags |= runner_abort_accepted;
        if (this->flags & runner_ref_work) {
            this->brick.abort();
        } else if (!(this->flags & runner_ref)) {
            delete this;
        }
    } else {
        this->flags |= runner_ref_work;
        this->on_update();
    }
}

template<typename Result, typename Reason, bool External>
void brick_runner<Result, Reason, External>::abort() {
    if (!(this->flags & runner_abort_requested)) {
        this->flags |= runner_abort_requested;
        if (!(this->flags & runner_ref_enqueue)) {
            this->flags |= runner_ref_enqueue;
            this->target.enqueue(static_cast<task&>(*this));
        }
    }
}

template<typename Result, typename Reason, bool External>
void brick_runner<Result, Reason, External>::drop_handle() {
    this->flags &= ~runner_ref_handle;
    if ((this->flags & runner_ref) == 0) {
        delete this;
    }
}

} // namespace internal

class handle {
public:
    explicit handle(internal::handleable * handleable): handleable(handleable) {}
    handle(handle const&) = delete;
    handle(handle && other):
        handleable(other.handleable)
    {
        other.handleable = nullptr;
    }

    ~handle() {
        if (this->handleable != nullptr) {
            this->handleable->drop_handle();
        }
    }

    handle & operator=(handle const&) = delete;
    handle & operator=(handle &&) = delete;

    void abort() {
        this->handleable->abort();
    }

    bool valid() const {
        return this->handleable != nullptr;
    }

private:
    internal::handleable * handleable;
};

template<typename Result, typename Reason>
inline handle enqueue(island & target, brick_ptr<Result, Reason> brick) {
    internal::handleable * handleable = new internal::brick_runner<Result, Reason, false>(target, std::move(brick));
    return handle(handleable);
}

template<typename Result, typename Reason>
inline void enqueue_external(island & target, brick_ptr<Result, Reason> brick) {
    internal::handleable * handleable = new internal::brick_runner<Result, Reason, true>(target, std::move(brick));
    handleable->drop_handle();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_RUNNER_H
