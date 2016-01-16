#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/blockFwd.h>

#include <abb/ll/valueTraits.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename BlockT = void, typename... ArgsT>
typename utils::Alternative<BlockT, Block<Und, typename ll::ArgsToValue<ArgsT...>::Type>>::Type error(ArgsT &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
