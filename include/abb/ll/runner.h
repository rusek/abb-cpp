#ifndef ABB_LL_RUNNER_H
#define ABB_LL_RUNNER_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

#include <abb/special.h>

namespace abb {
namespace ll {

namespace internal {

template<typename ResultT, typename ReasonT, bool External>
class Runner : private Successor, private Task {
public:
    Runner(Island & island, BrickPtr<ResultT, ReasonT> brick):
        island(island),
        brick(std::move(brick))
    {
        this->island.increfExternal();
        if (External) {
            this->island.enqueueExternal(static_cast<Task&>(*this));
        } else {
            this->island.enqueue(static_cast<Task&>(*this));
        }
    }

private:
    virtual void oncomplete();
    virtual Island & getIsland() const;
    virtual void run();

    Island & island;
    BrickPtr<ResultT, ReasonT> brick;
};

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::oncomplete() {
    this->island.decrefExternal();
    delete this;
}

template<typename ResultT, typename ReasonT, bool External>
Island & Runner<ResultT, ReasonT, External>::getIsland() const {
    return this->island;
}

template<typename ResultT, typename ReasonT, bool External>
void Runner<ResultT, ReasonT, External>::run() {
    this->brick.start(*this);
}

} // namespace internal

template<typename ResultT, typename ReasonT>
inline void enqueue(Island & island, BrickPtr<ResultT, ReasonT> brick) {
    new internal::Runner<ResultT, ReasonT, false>(island, std::move(brick));
}

template<typename ResultT, typename ReasonT>
inline void enqueueExternal(Island & island, BrickPtr<ResultT, ReasonT> brick) {
    new internal::Runner<ResultT, ReasonT, true>(island, std::move(brick));
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_RUNNER_H
