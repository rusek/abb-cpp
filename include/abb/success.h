#ifndef ABB_SUCCESS_H
#define ABB_SUCCESS_H

#include <abb/ll/valueBrick.h>
#include <abb/blockFwd.h>

#include <abb/ll/valueBrick.h>

#include <memory>

namespace abb {

template<typename BlockT, typename... ArgsT>
internal::SuccessReturn<BlockT, ArgsT...> success(ArgsT &&... args) {
    typedef internal::SuccessReturn<BlockT, ArgsT...> BlockType;
    typedef ll::ValueBrick<typename BlockType::ResultType, typename BlockType::ReasonType> ValueBrickType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        brick->setResult(std::forward<ArgsT>(args)...);
    } catch(...) {
        delete brick;
        throw;
    }
    return BlockType(std::unique_ptr<typename ValueBrickType::BrickType>(brick));
}


} // namespace abb

#endif