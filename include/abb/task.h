#ifndef ABB_TASK_H
#define ABB_TASK_H

#include <abb/utils/debug.h>

namespace abb {


class task {
public:
    task(): next(nullptr) {}

    virtual ~task() {}

    virtual void run() = 0;

private:
    task * next;

    friend class task_queue;
};

class task_queue {
public:
    task_queue(): last(nullptr) {}

    bool empty() const {
        return this->last == nullptr;
    }

    void push_back(task & queued) {
        ABB_ASSERT(queued.next == nullptr, "task already queued");
        if (this->last) {
            queued.next = this->last->next;
            this->last->next = &queued;

        } else {
            queued.next = &queued;
        }
        this->last = &queued;
    }

    task & pop_front() {
        ABB_ASSERT(this->last, "task queue is empty");
        task & popped = *this->last->next;
        if (popped.next == &popped) {
            this->last = nullptr;
        } else {
            this->last->next = popped.next;
        }
        popped.next = nullptr;
        return popped;
    }

private:
    task * last;
};


} // namespace abb

#endif // ABB_TASK_H
