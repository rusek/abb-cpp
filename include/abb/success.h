#ifndef ABB_SUCCESS_H
#define ABB_SUCCESS_H

#include <abb/ll/valueBrick.h>
#include <abb/blockFwd.h>

#include <abb/utils/alternative.h>

#include <memory>

namespace abb {

template<typename BlockT = void, typename... ArgsT>
typename utils::Alternative<BlockT, Block<void(ArgsT...)>>::Type success(ArgsT... args) {
    typedef typename utils::Alternative<BlockT, Block<void(ArgsT...)>>::Type BlockType;
    typedef ll::ValueBrick<typename BlockType::ResultType, typename BlockType::ReasonType> ValueBrickType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        brick->setResult(args...);
    } catch(...) {
        delete brick;
        throw;
    }
    return BlockType(std::unique_ptr<typename ValueBrickType::BrickType>(brick));
}


} // namespace abb

#endif