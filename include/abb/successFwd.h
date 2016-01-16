#ifndef ABB_SUCCESS_FWD_H
#define ABB_SUCCESS_FWD_H

#include <abb/blockFwd.h>

#include <abb/ll/valueTraits.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename BlockT = void, typename... ArgsT>
typename utils::Alternative<BlockT, Block<typename ll::ArgsToValue<ArgsT...>::Type>>::Type success(ArgsT &&... args);

} // namespace abb

#endif // ABB_SUCCESS_FWD_H
