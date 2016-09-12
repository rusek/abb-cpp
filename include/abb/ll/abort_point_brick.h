#ifndef ABB_LL_ABORT_POINT_BRICK_H
#define ABB_LL_ABORT_POINT_BRICK_H

#include <abb/ll/brick.h>

namespace abb {
namespace ll {

class abort_point_brick : public brick<void(), und_t>, private task {
public:
    abort_point_brick():
        cur_status(status::startable),
        aborted(false),
        succ(nullptr),
        result(box_arg) {}

    void start(island & target, bool aborted, successor & succ) { // TODO optimize aborted == true case
        this->succ = &succ;
        this->cur_status = status::running;
        this->aborted = aborted;
        target.enqueue_external(static_cast<task&>(*this));
    }

    void adopt(successor & succ) {
        this->succ = &succ;
    }

    void abort() {
        this->aborted = true;
    }

    status get_status() const {
        return this->cur_status;
    }

    store<void()> & get_result() {
        return this->result;
    }

private:
    virtual void run();

    status cur_status;
    bool aborted;
    successor * succ;
    store<void()> result;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ABORT_POINT_BRICK_H
