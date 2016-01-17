#ifndef ABB_ERROR_H
#define ABB_ERROR_H

#include <abb/errorFwd.h>
#include <abb/blockFwd.h>

#include <abb/ll/valueBrick.h>

#include <memory>

namespace abb {

template<typename BlockT, typename... ArgsT>
internal::ErrorReturn<BlockT, ArgsT...> error(ArgsT &&... args) {
    typedef internal::ErrorReturn<BlockT, ArgsT...> BlockType;
    typedef ll::ValueBrick<typename BlockType::ResultType, typename BlockType::ReasonType> ValueBrickType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        brick->setReason(std::forward<ArgsT>(args)...);
    } catch(...) {
        delete brick;
        throw;
    }
    return BlockType(std::unique_ptr<typename ValueBrickType::BrickType>(brick));
}

} // namespace abb

#endif // ABB_ERROR_H
