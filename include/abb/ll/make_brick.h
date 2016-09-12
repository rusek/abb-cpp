#ifndef ABB_LL_MAKE_BRICK_H
#define ABB_LL_MAKE_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason, typename Func>
class maker_brick : public brick<Result, Reason>, private successor {
public:
    template<typename... Args>
    maker_brick(Args &&... args): func(std::forward<Args>(args)...), succ(nullptr) {}

    void start(island & target, bool aborted, successor & succ) {
        this->target = &target;
        this->aborted = aborted;
        this->succ = &succ;
        this->wrapped_brick = std::move(this->func)();
        this->update();
    }

    void adopt(successor & succ) {
        this->succ = &succ;
    }

    void abort() {
        this->aborted = true;
        this->wrapped_brick.abort();
        this->update(); // TODO test
    }

    status get_status() const {
        if (this->wrapped_brick) {
            if (!this->target) {
                return status::next;
            } else {
                return status::running;
            }
        } else {
            return status::startable;
        }
    }

    brick_ptr<Result, Reason> get_next() {
        return std::move(this->wrapped_brick);
    }

private:
    virtual void on_update();

    void update();

    Func func;
    island * target;
    bool aborted;
    successor * succ;
    brick_ptr<Result, Reason> wrapped_brick;
};

template<typename Result, typename Reason, typename Func>
void maker_brick<Result, Reason, Func>::on_update() {
    this->update();
    if (!this->target) {
        this->succ->on_update();
    }
}

template<typename Result, typename Reason, typename Func>
void maker_brick<Result, Reason, Func>::update() {
    if (this->wrapped_brick.update(*this->target, this->aborted, *this) != status::running) {
        this->target = nullptr;
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_MAKE_BRICK_H
