#ifndef ABB_SUCCESS_FWD_H
#define ABB_SUCCESS_FWD_H

#include <abb/blockFwd.h>

namespace abb {

template<typename... ArgsT>
using SuccessBlock = Block<void(ArgsT...), Und>;

namespace internal {

template<typename BlockT, typename... ArgsT>
using SuccessReturn = typename std::conditional<
    IsPass<BlockT>::value,
    SuccessBlock<typename std::decay<ArgsT>::type...>,
    BlockT
>::type;

} // namespace internal

template<typename BlockT = Pass, typename... ArgsT>
internal::SuccessReturn<BlockT, ArgsT...> success(ArgsT &&... args);

} // namespace abb

#endif // ABB_SUCCESS_FWD_H
