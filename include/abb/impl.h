#ifndef ABB_IMPL_H
#define ABB_IMPL_H

#include <abb/reply.h>
#include <abb/ll/implBrick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/bridge.h>

#include <functional>

namespace abb {

template<typename BlockT, typename FuncT>
BlockT impl(FuncT && func) {
    typedef typename std::decay<FuncT>::type FuncD;
    typedef ll::ImplBrick<GetResult<BlockT>, GetReason<BlockT>, FuncD> ImplBrickType;

    return ll::packBrick<ImplBrickType>(std::forward<FuncT>(func));
}

} // namespace abb

#endif // ABB_IMPL_H
