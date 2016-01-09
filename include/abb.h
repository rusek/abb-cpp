#ifndef ABB_H
#define ABB_H

#include <abb/island.h>
#include <abb/ll/successBrick.h>
#include <abb/block.h>

namespace abb {

template<typename... Args>
Block<void(Args...)> success(Args... args) {
    typedef ll::SuccessBrick<void(Args...)> BrickType;

    BrickType * brick = new BrickType;
    try {
        brick->setResult(args...);
    } catch(...) {
        delete brick;
        throw;
    }
    return Block<void(Args...)>(std::unique_ptr<typename BrickType::BaseType>(brick));
}

} // namespace abb

#endif // ABB_H
