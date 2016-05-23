#ifndef ABB_SUCCESS_H
#define ABB_SUCCESS_H

#include <abb/successFwd.h>
#include <abb/blockFwd.h>

#include <abb/ll/successBrick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/bridge.h>

namespace abb {

template<typename BlockT, typename... ArgsT>
internal::SuccessReturn<BlockT, ArgsT...> success(ArgsT &&... args) {
    typedef internal::SuccessReturn<BlockT, ArgsT...> BlockType;
    typedef ll::SuccessBrick<GetResult<BlockType>, GetReason<BlockType>> SuccessBrickType;

    return ll::packBrick<SuccessBrickType>(std::forward<ArgsT>(args)...);
}


} // namespace abb

#endif // ABB_SUCCESS_H
