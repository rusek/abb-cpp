#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/block_fwd.h>

namespace abb {

namespace internal {

template<typename Block, typename... Args>
using error_return_t = typename std::conditional<
    is_pass<Block>::value,
    block<und_t, void(typename std::decay<Args>::type...)>,
    Block
>::type;

} // namespace internal

template<typename Block = pass_t, typename... Args>
internal::error_return_t<Block, Args...> error(Args &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
