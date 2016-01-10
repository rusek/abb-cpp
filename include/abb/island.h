#ifndef ABB_ISLAND_H
#define ABB_ISLAND_H

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <functional>
#include <deque>

#include <mutex>
#include <condition_variable>

namespace abb {

class Island : utils::Noncopyable {
public:
    Island();

    ~Island();

    void enqueue(std::function<void()> task);

    void enqueueExternal(std::function<void()> task);
    void increfExternal();
    void decrefExternal();

    void run();

    static Island & current();

private:
    std::deque<std::function<void()>> tasks;

    std::mutex mutex;
    std::condition_variable condition;
    std::deque<std::function<void()>> externalTasks;
    std::size_t externalCounter;

    static Island * currentPtr;
};

} // namespace abb

#endif // ABB_ISLAND_H
