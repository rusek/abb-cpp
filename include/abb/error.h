#ifndef ABB_ERROR_H
#define ABB_ERROR_H

#include <abb/errorFwd.h>
#include <abb/blockFwd.h>

#include <abb/ll/valueBrick.h>

#include <abb/utils/alternative.h>

namespace abb {

template<typename BlockT = void, typename... ArgsT>
typename utils::Alternative<BlockT, Block<Und, void(ArgsT...)>>::Type error(ArgsT... args) {
    typedef typename utils::Alternative<BlockT, Block<Und, void(ArgsT...)>>::Type BlockType;
    typedef ll::ValueBrick<typename BlockType::ResultType, typename BlockType::ReasonType> ValueBrickType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        brick->setReason(args...);
    } catch(...) {
        delete brick;
        throw;
    }
    return BlockType(std::unique_ptr<typename ValueBrickType::BrickType>(brick));
}

} // namespace abb

#endif // ABB_ERROR_H
