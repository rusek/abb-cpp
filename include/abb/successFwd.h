#ifndef ABB_SUCCESS_FWD_H
#define ABB_SUCCESS_FWD_H

#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename BlockT = void, typename... ArgsT>
typename utils::Alternative<BlockT, Block<void(ArgsT...)>>::Type success(ArgsT... args);

} // namespace abb

#endif // ABB_SUCCESS_FWD_H
