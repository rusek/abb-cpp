#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/blockFwd.h>

namespace abb {

namespace internal {

template<typename BlockT, typename... ArgsT>
using ErrorReturn = typename std::conditional<
    IsPass<BlockT>::value,
    Block<Und, void(typename std::decay<ArgsT>::type...)>,
    BlockT
>::type;

} // namespace internal

template<typename BlockT = Pass, typename... ArgsT>
internal::ErrorReturn<BlockT, ArgsT...> error(ArgsT &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
