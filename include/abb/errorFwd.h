#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

namespace abb {

namespace internal {

template<typename BlockT, typename... ArgsT>
struct ErrorReturn {
    typedef utils::AlternativeT<BlockT, Block<Und, void(typename std::decay<ArgsT>::type...)>> Type;
};

template<typename BlockT, typename... ArgsT>
using ErrorReturnT = typename ErrorReturn<BlockT, ArgsT...>::Type;

} // namespace internal

template<typename BlockT = void, typename... ArgsT>
internal::ErrorReturnT<BlockT, ArgsT...> error(ArgsT &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
