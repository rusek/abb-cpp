#ifndef ABB_ISLAND_H
#define ABB_ISLAND_H

#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>

#include <functional>
#include <deque>

namespace abb {

class Island : utils::Noncopyable {
public:
    Island();

    ~Island();

    void enqueue(std::function<void()> task);

    void run();

    static Island & current();

private:
    std::deque<std::function<void()>> tasks;

    static Island * currentPtr;
};

} // namespace abb

#endif // ABB_ISLAND_H
