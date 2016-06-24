#ifndef ABB_MAKE_H
#define ABB_MAKE_H

#include <abb/ll/make_brick.h>
#include <abb/ll/bridge.h>

#include <abb/block.h>
#include <abb/make_fwd.h>

namespace abb {

template<typename Func, typename... Args>
internal::make_return_t<Func> make(Args &&... args) {
    typedef internal::make_return_t<Func> block_type;
    typedef ll::maker_brick<
        get_result_t<block_type>,
        get_reason_t<block_type>,
        ll::unpacker<Func>
    > maker_brick_type;

    return ll::pack_brick<maker_brick_type>(std::forward<Args>(args)...);
}

} // namespace abb

#endif // ABB_MAKE_H
