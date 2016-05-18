#ifndef ABB_LL_RUNNER_H
#define ABB_LL_RUNNER_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/special.h>

namespace abb {
namespace ll {

namespace internal {

class Handleable {
public:
    virtual void abort() = 0;
    virtual void dropHandle() = 0;
};

enum {
    RUNNER_PENDING = 0,
    RUNNER_ABORTING = 1,
    RUNNER_COMPLETED = 2,
    RUNNER_HANDLE_DROPPED = 4,
    RUNNER_STARTED = 8
};

typedef int Flags;

template<typename ResultT, typename ReasonT, bool External>
class Runner : public Handleable, private Successor, private Task {
public:
    Runner(Island & island, BrickPtr<ResultT, ReasonT> brick):
        island(island),
        brick(std::move(brick)),
        flags(RUNNER_PENDING)
    {
        this->island.increfExternal();
        if (External) {
            this->island.enqueueExternal(static_cast<Task&>(*this));
        } else {
            this->island.enqueue(static_cast<Task&>(*this));
        }
    }

    virtual void abort();
    virtual void dropHandle();

private:
    virtual void oncomplete();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;
    virtual void run();

    Island & island;
    BrickPtr<ResultT, ReasonT> brick;
    Flags flags;
};

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::oncomplete() {
    this->island.decrefExternal();
    this->flags |= RUNNER_COMPLETED;
    if (this->flags & RUNNER_HANDLE_DROPPED) {
        delete this;
    }
}

template<typename ResultT, typename ReasonT, bool External>
Island & Runner<ResultT, ReasonT, External>::getIsland() const {
    return this->island;
}

template<typename ResultT, typename ReasonT, bool External>
bool Runner<ResultT, ReasonT, External>::isAborted() const {
    return (this->flags & RUNNER_ABORTING) != 0;
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::run() {
    this->brick.start(*this);
    // FIXME handle abort ????
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::abort() {
    if (!(this->flags & RUNNER_ABORTING)) {
        this->flags |= RUNNER_ABORTING;
        if (this->flags & RUNNER_STARTED) {
            this->brick.abort(); // FIXME should we enqueue it ????
        }
    }
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::dropHandle() {
    this->flags |= RUNNER_HANDLE_DROPPED;
    if (this->flags & RUNNER_COMPLETED) {
        delete this;
    }
}

} // namespace internal

class Handle {
public:
    explicit Handle(internal::Handleable * handleable): handleable(handleable) {}
    Handle(Handle const&) = delete;
    Handle(Handle && other):
        handleable(other.handleable)
    {
        other.handleable = nullptr;
    }

    ~Handle() {
        if (this->handleable != nullptr) {
            this->handleable->dropHandle();
        }
    }

    Handle & operator=(Handle const&) = delete;
    Handle & operator=(Handle &&) = delete;

    void abort() {
        this->handleable->abort();
    }

    bool valid() const {
        return this->handleable != nullptr;
    }

private:
    internal::Handleable * handleable;
};

template<typename ResultT, typename ReasonT>
inline Handle enqueue(Island & island, BrickPtr<ResultT, ReasonT> brick) {
    internal::Handleable * handleable = new internal::Runner<ResultT, ReasonT, false>(island, std::move(brick));
    return Handle(handleable);
}

template<typename ResultT, typename ReasonT>
inline void enqueueExternal(Island & island, BrickPtr<ResultT, ReasonT> brick) {
    internal::Handleable * handleable = new internal::Runner<ResultT, ReasonT, true>(island, std::move(brick));
    handleable->dropHandle();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_RUNNER_H
