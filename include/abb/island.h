#ifndef ABB_ISLAND_H
#define ABB_ISLAND_H

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <abb/task.h>

#include <functional>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <type_traits>

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

    void increfExternal();
    void decrefExternal();

    void run();

    static Island & current();

private:
    void enqueueTask(Task & task);
    void enqueueTaskExternal(Task & task);

    TaskQueue tasks;

    std::mutex mutex;
    std::condition_variable condition;
    TaskQueue externalTasks;
    std::size_t externalCounter;

    static Island * currentPtr;

    friend void enqueue(Island & island, Task & task) {
        island.enqueueTask(task);
    }

    friend void enqueueExternal(Island & island, Task & task) {
        island.enqueueTaskExternal(task);
    }
};

namespace internal {

template<typename FuncT>
class FunctorTask : public Task {
public:
    explicit FunctorTask(FuncT && func): func(std::forward<FuncT>(func)) {}
    explicit FunctorTask(FuncT const& func): func(func) {}

    virtual void run();

private:
    std::function<void()> func;
};

template<typename FuncT>
void FunctorTask<FuncT>::run() {
    this->func();
    delete this;
}

} // namespace internal

template<
    typename FuncT,
    typename std::enable_if<std::is_same<typename std::result_of<FuncT()>::type, void>::value>::type* = nullptr
>
void enqueue(Island & island, FuncT && func) {
    Task * task = new internal::FunctorTask<typename std::decay<FuncT>::type>(func);
    island.enqueue(*task);
}

template<
    typename FuncT,
    typename std::enable_if<std::is_same<typename std::result_of<FuncT()>::type, void>::value>::type* = nullptr
>
void enqueueExternal(Island & island, FuncT && func) {
    Task * task = new internal::FunctorTask<typename std::decay<FuncT>::type>(func);
    island.enqueueExternal(*task);
}

} // namespace abb

#endif // ABB_ISLAND_H
