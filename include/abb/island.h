#ifndef ABB_ISLAND_H
#define ABB_ISLAND_H

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <abb/task.h>

#include <functional>
#include <deque>

#include <mutex>
#include <condition_variable>

namespace abb {

class Island;

namespace internal {

template<typename ArgT>
inline auto doEnqueue(Island & island, ArgT && arg) ->
    decltype(enqueue(island, std::forward<ArgT>(arg)))
{
    return enqueue(island, std::forward<ArgT>(arg));
}

template<typename ArgT>
inline auto doEnqueueExternal(Island & island, ArgT && arg) ->
    decltype(enqueueExternal(island, std::forward<ArgT>(arg)))
{
    return enqueueExternal(island, std::forward<ArgT>(arg));
}


} // namespace internal

class Island : utils::Noncopyable {
public:
    Island();

    ~Island();

    template<typename ArgT>
    auto enqueue(ArgT && arg) -> decltype(internal::doEnqueue(*this, std::forward<ArgT>(arg))) {
        return internal::doEnqueue(*this, std::forward<ArgT>(arg));
    }

    template<typename ArgT>
    auto enqueueExternal(ArgT && arg) -> decltype(internal::doEnqueueExternal(*this, std::forward<ArgT>(arg))) {
        return internal::doEnqueueExternal(*this, std::forward<ArgT>(arg));
    }

    void enqueue2(std::function<void()> task);
    void enqueue2(Task & task);

    void enqueueExternal2(std::function<void()> task);
    void enqueueExternal2(Task & task);
    void increfExternal();
    void decrefExternal();

    void run();

    static Island & current();

private:
    TaskQueue tasks;

    std::mutex mutex;
    std::condition_variable condition;
    TaskQueue externalTasks;
    std::size_t externalCounter;

    static Island * currentPtr;
};

inline void enqueue(Island & island, Task & task) {
    island.enqueue2(task);
}

inline void enqueueExternal(Island & island, Task & task) {
    island.enqueueExternal2(task);
}

inline void enqueue(Island & island, std::function<void()> task) {
    island.enqueue2(task);
}

inline void enqueueExternal(Island & island, std::function<void()> task) {
    island.enqueueExternal2(task);
}


} // namespace abb

#endif // ABB_ISLAND_H
