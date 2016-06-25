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

    void start(successor & succ) {
        this->succ = &succ;
        this->wrapped_brick = std::move(this->func)();
        this->on_update();
    }

    void abort() {
        this->wrapped_brick.abort();
    }

    status get_status() const {
        return (
            this->wrapped_brick &&
            (this->wrapped_brick.get_status() & (success_status | error_status | abort_status))
        ) ? next_status : pending_status;
    }

    brick_ptr<Result, Reason> get_next() {
        return std::move(this->wrapped_brick);
    }

private:
    virtual void on_update();
    virtual island & get_island() const;
    virtual bool is_aborted() const;

    Func func;
    successor * succ;
    brick_ptr<Result, Reason> wrapped_brick;
};

template<typename Result, typename Reason, typename Func>
void maker_brick<Result, Reason, Func>::on_update() {
    if (this->wrapped_brick.try_start(*this)) {
        this->succ->on_update();
    }
}

template<typename Result, typename Reason, typename Func>
island & maker_brick<Result, Reason, Func>::get_island() const {
    return this->succ->get_island();
}

template<typename Result, typename Reason, typename Func>
bool maker_brick<Result, Reason, Func>::is_aborted() const {
    return this->succ->is_aborted();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_MAKE_BRICK_H
