#ifndef ABB_MAKE_H
#define ABB_MAKE_H

#include <abb/ll/makeBrick.h>
#include <abb/block.h>
#include <abb/makeFwd.h>

namespace abb {

template<typename FuncT, typename... ArgsT>
internal::MakeReturn<FuncT> make(ArgsT &&... args) {
    typedef internal::MakeReturn<FuncT> BlockType;
    typedef ll::MakeBrick<
        GetResult<BlockType>,
        GetReason<BlockType>,
        typename BlockType::template Unpacker<FuncT>
    > MakeBrickType;

    return BlockType(ll::makeBrick<MakeBrickType>(std::forward<ArgsT>(args)...));
}

} // namespace abb

#endif // ABB_MAKE_H
