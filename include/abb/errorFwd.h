#ifndef ABB_ERROR_FWD_H
#define ABB_ERROR_FWD_H

#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename BlockT = void, typename... ArgsT>
typename utils::Alternative<BlockT, Block<Und, void(typename std::decay<ArgsT>::type...)>>::Type error(ArgsT &&... args);

} // namespace abb

#endif // ABB_ERROR_FWD_H
