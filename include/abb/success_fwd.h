#ifndef ABB_SUCCESS_FWD_H
#define ABB_SUCCESS_FWD_H

#include <abb/block_fwd.h>

namespace abb {

namespace internal {

template<typename Block, typename... Args>
using success_return_t = typename std::conditional<
    is_pass<Block>::value,
    block<void(typename std::decay<Args>::type...), und_t>,
    Block
>::type;

} // namespace internal

template<typename Block = pass_t, typename... Args>
internal::success_return_t<Block, Args...> success(Args &&... args);

} // namespace abb

#endif // ABB_SUCCESS_FWD_H
