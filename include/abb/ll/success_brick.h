#ifndef ABB_LL_SUCCESS_BRICK_H
#define ABB_LL_SUCCESS_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

template<typename Result>
class success_brick : public brick<Result, und_t> {
public:
    template<typename... Args>
    explicit success_brick(Args &&... args):
        value(box_arg, std::forward<Args>(args)...) {}

    status get_status() const {
        return status::success;
    }

    store<Result> & get_result() {
        return this->value;
    }

private:
    store<Result> value;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H
