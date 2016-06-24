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

class island;

namespace internal {

template<typename Arg>
inline auto do_enqueue(island & target, Arg && arg) ->
    decltype(enqueue(target, std::forward<Arg>(arg)))
{
    return enqueue(target, std::forward<Arg>(arg));
}

template<typename Arg>
inline auto do_enqueue_external(island & target, Arg && arg) ->
    decltype(enqueue_external(target, std::forward<Arg>(arg)))
{
    return enqueue_external(target, std::forward<Arg>(arg));
}


} // namespace internal

class island : utils::noncopyable {
public:
    island();

    ~island();

    template<typename Arg>
    auto enqueue(Arg && arg) -> decltype(internal::do_enqueue(*this, std::forward<Arg>(arg))) {
        return internal::do_enqueue(*this, std::forward<Arg>(arg));
    }

    template<typename Arg>
    auto enqueue_external(Arg && arg) -> decltype(internal::do_enqueue_external(*this, std::forward<Arg>(arg))) {
        return internal::do_enqueue_external(*this, std::forward<Arg>(arg));
    }

    void incref_external();
    void decref_external();

    void run();

    static island & current();

private:
    void enqueue_task(task & to_enqueue);
    void enqueue_task_external(task & to_enqueue);

    task_queue tasks;

    std::mutex mutex;
    std::condition_variable condition;
    task_queue external_tasks;
    std::size_t external_counter;

    static island * current_ptr;

    friend void enqueue(island & target, task & to_enqueue) {
        target.enqueue_task(to_enqueue);
    }

    friend void enqueue_external(island & target, task & to_enqueue) {
        target.enqueue_task_external(to_enqueue);
    }
};

namespace internal {

template<typename Func>
class functor_task : public task {
public:
    explicit functor_task(Func && func): func(std::forward<Func>(func)) {}
    explicit functor_task(Func const& func): func(func) {}

    virtual void run();

private:
    std::function<void()> func;
};

template<typename Func>
void functor_task<Func>::run() {
    this->func();
    delete this;
}

} // namespace internal

template<
    typename Func,
    typename std::enable_if<std::is_same<typename std::result_of<Func()>::type, void>::value>::type* = nullptr
>
void enqueue(island & target, Func && func) {
    task * to_enqueue = new internal::functor_task<typename std::decay<Func>::type>(func);
    target.enqueue(*to_enqueue);
}

template<
    typename Func,
    typename std::enable_if<std::is_same<typename std::result_of<Func()>::type, void>::value>::type* = nullptr
>
void enqueue_external(island & target, Func && func) {
    task * to_enqueue = new internal::functor_task<typename std::decay<Func>::type>(func);
    target.enqueue_external(*to_enqueue);
}

} // namespace abb

#endif // ABB_ISLAND_H
