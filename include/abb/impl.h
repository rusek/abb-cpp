#ifndef ABB_IMPL_H
#define ABB_IMPL_H

#include <abb/reply.h>
#include <abb/ll/impl_brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/bridge.h>

#include <functional>

namespace abb {

template<typename Block, typename Func>
Block impl(Func && func) {
    typedef ll::impl_brick<
        get_result_t<Block>,
        get_reason_t<Block>,
        typename std::decay<Func>::type
    > impl_brick_type;

    return ll::pack_brick<impl_brick_type>(std::forward<Func>(func));
}

} // namespace abb

#endif // ABB_IMPL_H
