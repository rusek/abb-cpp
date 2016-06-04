#ifndef ABB_LL_RUNNER_H
#define ABB_LL_RUNNER_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/value.h>

namespace abb {
namespace ll {

namespace internal {

class Handleable {
public:
    virtual void abort() = 0;
    virtual void dropHandle() = 0;
};

enum {
    RUNNER_REF_HANDLE = 1,
    RUNNER_REF_ENQUEUE = 2,
    RUNNER_REF_WORK = 4,
    RUNNER_REF = RUNNER_REF_HANDLE | RUNNER_REF_ENQUEUE | RUNNER_REF_WORK,
    RUNNER_ABORT_REQUESTED = 8,
    RUNNER_ABORT_ACCEPTED = 16
};

typedef int Flags;

template<typename ResultT, typename ReasonT, bool External>
class Runner : public Handleable, private Successor, private Task {
public:
    Runner(Island & island, BrickPtr<ResultT, ReasonT> brick):
        island(island),
        brick(std::move(brick)),
        flags(RUNNER_REF_ENQUEUE | RUNNER_REF_HANDLE)
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
    virtual void onUpdate();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;
    virtual void run();

    Island & island;
    BrickPtr<ResultT, ReasonT> brick;
    Flags flags;
};

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::onUpdate() {
    for (;;) {
        Status status = this->brick.getStatus();
        if (status == PENDING) {
            this->brick.start(*this);
            return;
        } else if (status & NEXT) {
            this->brick = this->brick.getNext();
        } else {
            this->island.decrefExternal();
            this->flags &= ~RUNNER_REF_WORK;
            if ((this->flags & RUNNER_REF) == 0) {
                delete this;
            }
            return;
        }
    }
}

template<typename ResultT, typename ReasonT, bool External>
Island & Runner<ResultT, ReasonT, External>::getIsland() const {
    return this->island;
}

template<typename ResultT, typename ReasonT, bool External>
bool Runner<ResultT, ReasonT, External>::isAborted() const {
    return (this->flags & RUNNER_ABORT_ACCEPTED) != 0;
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::run() {
    this->flags &= ~RUNNER_REF_ENQUEUE;
    if (this->flags & RUNNER_ABORT_REQUESTED) {
        this->flags |= RUNNER_ABORT_ACCEPTED;
        if (this->flags & RUNNER_REF_WORK) {
            this->brick.abort();
        } else if (!(this->flags & RUNNER_REF)) {
            delete this;
        }
    } else {
        this->flags |= RUNNER_REF_WORK;
        this->onUpdate();
    }
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::abort() {
    if (!(this->flags & RUNNER_ABORT_REQUESTED)) {
        this->flags |= RUNNER_ABORT_REQUESTED;
        if (!(this->flags & RUNNER_REF_ENQUEUE)) {
            this->flags |= RUNNER_REF_ENQUEUE;
            this->island.enqueue(static_cast<Task&>(*this));
        }
    }
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::dropHandle() {
    this->flags &= ~RUNNER_REF_HANDLE;
    if ((this->flags & RUNNER_REF) == 0) {
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
