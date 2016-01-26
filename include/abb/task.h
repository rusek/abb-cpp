#ifndef ABB_TASK_H
#define ABB_TASK_H

#include <abb/utils/debug.h>

namespace abb {


class Task {
public:
    Task(): next(nullptr) {}

    virtual ~Task() {}

    virtual void run() = 0;

private:
    Task * next;

    friend class TaskQueue;
};

class TaskQueue {
public:
    TaskQueue(): last(nullptr) {}

    bool empty() const {
        return this->last == nullptr;
    }

    void pushBack(Task & task) {
        ABB_ASSERT(task.next == nullptr, "Task already queued");
        if (this->last) {
            task.next = this->last->next;
            this->last->next = &task;

        } else {
            task.next = &task;
        }
        this->last = &task;
    }

    Task & popFront() {
        ABB_ASSERT(this->last, "Task queue is empty");
        Task & task = *this->last->next;
        if (task.next == &task) {
            this->last = nullptr;
        } else {
            this->last->next = task.next;
        }
        task.next = nullptr;
        return task;
    }

private:
    Task * last;
};


} // namespace abb

#endif // ABB_TASK_H
