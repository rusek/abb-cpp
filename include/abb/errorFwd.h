#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/blockFwd.h>

namespace abb {

template<typename... ArgsT>
using ErrorBlock = Block<Und, void(ArgsT...)>;

namespace internal {

template<typename BlockT, typename... ArgsT>
using ErrorReturn = typename std::conditional<
    IsPass<BlockT>::value,
    ErrorBlock<typename std::decay<ArgsT>::type...>,
    BlockT
>::type;

} // namespace internal

template<typename BlockT = Pass, typename... ArgsT>
internal::ErrorReturn<BlockT, ArgsT...> error(ArgsT &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
