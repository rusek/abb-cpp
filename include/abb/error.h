#ifndef ABB_ERROR_H
#define ABB_ERROR_H

#include <abb/errorFwd.h>
#include <abb/blockFwd.h>

#include <abb/ll/errorBrick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/bridge.h>

namespace abb {

template<typename BlockT, typename... ArgsT>
internal::ErrorReturn<BlockT, ArgsT...> error(ArgsT &&... args) {
    typedef internal::ErrorReturn<BlockT, ArgsT...> BlockType;
    typedef ll::ErrorBrick<GetResult<BlockType>, GetReason<BlockType>> ErrorBrickType;

    return ll::packBrick<ErrorBrickType>(std::forward<ArgsT>(args)...);
}

} // namespace abb

#endif // ABB_ERROR_H
