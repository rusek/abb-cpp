#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename... ArgsT>
using ErrorBlock = Block<Und, void(ArgsT...)>;

namespace internal {

template<typename BlockT, typename... ArgsT>
using ErrorReturn = utils::Alternative<BlockT, ErrorBlock<typename std::decay<ArgsT>::type...>>;

} // namespace internal

template<typename BlockT = void, typename... ArgsT>
internal::ErrorReturn<BlockT, ArgsT...> error(ArgsT &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
