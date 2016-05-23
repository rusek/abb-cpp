#ifndef ABB_LL_BRIDGE_H
#define ABB_LL_BRIDGE_H

#include <abb/blockFwd.h>
#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
inline BaseBlock<ResultT, ReasonT> packBrickPtr(BrickPtr<ResultT, ReasonT> && brick) {
    return BaseBlock<ResultT, ReasonT>(std::move(brick));
}

template<typename ResultT, typename ReasonT>
inline BrickPtr<ResultT, ReasonT> unpackBrickPtr(BaseBlock<ResultT, ReasonT> && block) {
    return block.takeBrick();
}

template<typename BrickT, typename... ArgsT>
inline GetBlock<BrickT> packBrick(ArgsT &&... args) {
    return packBrickPtr(makeBrick<BrickT>(std::forward<ArgsT>(args)...));
}

template<typename ContT>
class Unpacker {
private:
    ContT cont; // must be present before operator()

public:
    template<typename... ArgsT>
    explicit Unpacker(ArgsT &&... args): cont(std::forward<ArgsT>(args)...) {}

    template<typename... ArgsT>
    auto operator()(ArgsT &&... args) -> decltype(unpackBrickPtr(this->cont(std::forward<ArgsT>(args)...))) {
        return unpackBrickPtr(this->cont(std::forward<ArgsT>(args)...));
    }

};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRIDGE_H
