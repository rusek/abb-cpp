#ifndef ABB_LL_ABORT_POINT_BRICK_H
#define ABB_LL_ABORT_POINT_BRICK_H

#include <abb/ll/brick.h>

namespace abb {
namespace ll {

class abort_point_brick : public brick<void(), und_t>, private task {
public:
    abort_point_brick():
        cur_status(status::startable),
        succ(nullptr),
        result(box_arg) {}

    void start(successor & succ) {
        this->succ = &succ;
        this->cur_status = status::running;
        this->succ->get_island().enqueue_external(static_cast<task&>(*this));
    }

    void adopt(successor & succ) {
        this->succ = &succ;
    }

    void abort() {}

    status get_status() const {
        return this->cur_status;
    }

    store<void()> & get_result() {
        return this->result;
    }

private:
    virtual void run();

    status cur_status;
    successor * succ;
    store<void()> result;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ABORT_POINT_BRICK_H
