#ifndef ABB_SUCCESS_FWD_H
#define ABB_SUCCESS_FWD_H

#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

namespace abb {

namespace internal {

template<typename BlockT, typename... ArgsT>
struct SuccessReturn {
    typedef utils::AlternativeT<BlockT, Block<void(typename std::decay<ArgsT>::type...)>> Type;
};

template<typename BlockT, typename... ArgsT>
using SuccessReturnT = typename SuccessReturn<BlockT, ArgsT...>::Type;

} // namespace internal

template<typename BlockT = void, typename... ArgsT>
internal::SuccessReturnT<BlockT, ArgsT...> success(ArgsT &&... args);

} // namespace abb

#endif // ABB_SUCCESS_FWD_H
