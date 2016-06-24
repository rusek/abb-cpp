#ifndef ABB_LL_SUCCESS_BRICK_H
#define ABB_LL_SUCCESS_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class success_brick : public brick<Result, Reason> {
public:
    typedef Result result;
    typedef Reason reason;

    template<typename... Args>
    explicit success_brick(Args &&... args):
        value(std::forward<Args>(args)...) {}

    void abort() {}

    void start(successor &) {
        ABB_FIASCO("start called on success_brick");
    }

    status get_status() const {
        return success_status;
    }

    store<Result> & get_result() {
        return this->value;
    }

    store<Reason> & get_reason() {
        ABB_FIASCO("get_reason called on success_brick");
    }

private:
    store<result> value;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H
