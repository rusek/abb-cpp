#include <abb/island.h>
#include <abb/utils/debug.h>

namespace abb {

Island * Island::currentPtr = nullptr;

Island::Island() {}

Island::~Island() {}

void Island::enqueue(std::function<void()> task) {
    this->tasks.push_back(task);
}

void Island::run() {
    ABB_ASSERT(Island::currentPtr == nullptr, "Current island already set");
    Island::currentPtr = this;

    while (!this->tasks.empty()) {
        std::function<void()> task = this->tasks.front();
        this->tasks.pop_front();
        task();
    }

    ABB_ASSERT(Island::currentPtr == this, "Current island changed in the meantime");
    Island::currentPtr = nullptr;
}

Island & Island::current() {
    ABB_ASSERT(Island::currentPtr != nullptr, "Current island not set");
    return *Island::currentPtr;
}

} // namespace abb
