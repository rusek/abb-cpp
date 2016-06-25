#ifndef ABB_LL_ERROR_BRICK_H
#define ABB_LL_ERROR_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

template<typename Reason>
class error_brick : public brick<und_t, Reason> {
public:
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

    store<Reason> & get_reason() {
        return this->value;
    }

private:
    store<Reason> value;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ERROR_BRICK_H
