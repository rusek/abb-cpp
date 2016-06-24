#ifndef ABB_LL_ERROR_BRICK_H
#define ABB_LL_ERROR_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class error_brick : public brick<Result, Reason> {
public:
    typedef Result result;
    typedef Reason reason;

    template<typename... Args>
    explicit error_brick(Args &&... args):
        value(std::forward<Args>(args)...) {}

    void abort() {}

    void start(successor &) {
        ABB_FIASCO("start called on error_brick");
    }

    status get_status() const {
        return error_status;
    }

    store<Result> & get_result() {
        ABB_FIASCO("get_result called on error_brick");
    }

    store<Reason> & get_reason() {
        return this->value;
    }

private:
    store<reason> value;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ERROR_BRICK_H
