#include <abb/island.h>
#include <abb/utils/debug.h>

namespace abb {

Island * Island::currentPtr = nullptr;

Island::Island(): externalCounter(0) {}

Island::~Island() {}

void Island::enqueue(std::function<void()> task) {
    ABB_ASSERT(Island::currentPtr == this, "Not a current island");
    this->tasks.push_back(task);
}


void Island::enqueueExternal(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(this->mutex);
    bool wasEmpty = this->externalTasks.empty();
    this->externalTasks.push_back(task);
    if (wasEmpty) {
        this->condition.notify_one();
    }
}

void Island::increfExternal() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->externalCounter++;
}

void Island::decrefExternal() {
    std::unique_lock<std::mutex> lock(this->mutex);
    ABB_ASSERT(this->externalCounter > 0, "External counter is zero");
    this->externalCounter--;
    if (!this->externalCounter) {
        this->condition.notify_one();
    }
}

void Island::run() {
    ABB_ASSERT(Island::currentPtr == nullptr, "Current island already set");
    Island::currentPtr = this;

    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            while (this->externalCounter > 0 && this->externalTasks.empty()) {
                this->condition.wait(lock);
            }

            if (this->externalTasks.empty()) {
                break;
            }

            task = this->externalTasks.front();
            this->externalTasks.pop_front();
        }
        task();

        while (!this->tasks.empty()) {
            std::function<void()> task = this->tasks.front();
            this->tasks.pop_front();
            task();
        }
    }

    ABB_ASSERT(Island::currentPtr == this, "Current island changed in the meantime");
    Island::currentPtr = nullptr;
}

Island & Island::current() {
    ABB_ASSERT(Island::currentPtr != nullptr, "Current island not set");
    return *Island::currentPtr;
}

} // namespace abb
