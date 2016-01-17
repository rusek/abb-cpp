#ifndef ABB_SUCCESS_FWD_H
#define ABB_SUCCESS_FWD_H

#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename... ArgsT>
using SuccessBlock = Block<void(ArgsT...), Und>;

namespace internal {

template<typename BlockT, typename... ArgsT>
using SuccessReturn = utils::Alternative<BlockT, SuccessBlock<typename std::decay<ArgsT>::type...>>;

} // namespace internal

template<typename BlockT = void, typename... ArgsT>
internal::SuccessReturn<BlockT, ArgsT...> success(ArgsT &&... args);

} // namespace abb

#endif // ABB_SUCCESS_FWD_H
