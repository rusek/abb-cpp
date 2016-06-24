#ifndef ABB_SUCCESS_H
#define ABB_SUCCESS_H

#include <abb/success_fwd.h>
#include <abb/block_fwd.h>

#include <abb/ll/success_brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/bridge.h>

namespace abb {

template<typename Block, typename... Args>
internal::success_return_t<Block, Args...> success(Args &&... args) {
    typedef internal::success_return_t<Block, Args...> block_type;
    typedef ll::success_brick<get_result_t<block_type>, get_reason_t<block_type>> success_brick_type;

    return ll::pack_brick<success_brick_type>(std::forward<Args>(args)...);
}


} // namespace abb

#endif // ABB_SUCCESS_H
