#ifndef ABB_ERROR_H
#define ABB_ERROR_H

#include <abb/error_fwd.h>
#include <abb/block_fwd.h>

#include <abb/ll/error_brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/bridge.h>

namespace abb {

template<typename Block, typename... Args>
internal::error_return_t<Block, Args...> error(Args &&... args) {
    typedef internal::error_return_t<Block, Args...> block_type;
    typedef ll::error_brick<get_result_t<block_type>, get_reason_t<block_type>> error_brick_type;

    return ll::pack_brick<error_brick_type>(std::forward<Args>(args)...);
}

} // namespace abb

#endif // ABB_ERROR_H
