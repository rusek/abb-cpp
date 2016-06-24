#include <abb/island.h>
#include <abb/utils/debug.h>

namespace abb {

island * island::current_ptr = nullptr;

island::island(): external_counter(0) {}

island::~island() {}

void island::enqueue_task(task & to_enqueue) {
    ABB_ASSERT(island::current_ptr == this, "Not a current island");
    this->tasks.push_back(to_enqueue);
}

void island::enqueue_task_external(task & to_enqueue) {
    std::unique_lock<std::mutex> lock(this->mutex);
    bool was_empty = this->external_tasks.empty();
    this->external_tasks.push_back(to_enqueue);
    if (was_empty) {
        this->condition.notify_one();
    }
}

void island::incref_external() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->external_counter++;
}

void island::decref_external() {
    std::unique_lock<std::mutex> lock(this->mutex);
    ABB_ASSERT(this->external_counter > 0, "External counter is zero");
    this->external_counter--;
    if (!this->external_counter) {
        this->condition.notify_one();
    }
}

void island::run() {
    ABB_ASSERT(island::current_ptr == nullptr, "Current island already set");
    island::current_ptr = this;

    while (true) {
        task * to_run;
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            while (this->external_counter > 0 && this->external_tasks.empty()) {
                this->condition.wait(lock);
            }

            if (this->external_tasks.empty()) {
                break;
            }

            to_run = &this->external_tasks.pop_front();
        }
        to_run->run();

        while (!this->tasks.empty()) {
            this->tasks.pop_front().run();
        }
    }

    ABB_ASSERT(island::current_ptr == this, "Current island changed in the meantime");
    island::current_ptr = nullptr;
}

island & island::current() {
    ABB_ASSERT(island::current_ptr != nullptr, "Current island not set");
    return *island::current_ptr;
}

} // namespace abb
