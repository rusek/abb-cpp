#ifndef ABB_IMPL_H
#define ABB_IMPL_H

#include <abb/reply.h>
#include <abb/ll/implBrick.h>

#include <functional>

namespace abb {

template<typename BlockT, typename FuncT>
BlockT impl(FuncT && func) {
    typedef typename std::decay<FuncT>::type FuncD;
    typedef ll::ImplBrick<typename BlockT::ResultType, typename BlockT::ReasonType, FuncD> ImplBrickType;

    return BlockT(ll::makeBrick<ImplBrickType>(std::forward<FuncT>(func)));
}

} // namespace abb

#endif // ABB_IMPL_H
