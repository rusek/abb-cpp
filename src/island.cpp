#include <abb/island.h>
#include <abb/utils/debug.h>

namespace abb {

namespace {

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

} // namespace

Island * Island::currentPtr = nullptr;

Island::Island(): externalCounter(0) {}

Island::~Island() {}

void Island::enqueue(std::function<void()> task) {
    this->enqueue(*(new FunctorTask<std::function<void()>>(task)));
}

void Island::enqueue(Task & task) {
    ABB_ASSERT(Island::currentPtr == this, "Not a current island");
    this->tasks.pushBack(task);
}

void Island::enqueueExternal(std::function<void()> task) {
    this->enqueueExternal(*(new FunctorTask<std::function<void()>>(task)));
}

void Island::enqueueExternal(Task & task) {
    std::unique_lock<std::mutex> lock(this->mutex);
    bool wasEmpty = this->externalTasks.empty();
    this->externalTasks.pushBack(task);
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
        Task * task;
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            while (this->externalCounter > 0 && this->externalTasks.empty()) {
                this->condition.wait(lock);
            }

            if (this->externalTasks.empty()) {
                break;
            }

            task = &this->externalTasks.popFront();
        }
        task->run();

        while (!this->tasks.empty()) {
            this->tasks.popFront().run();
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
