#ifndef ABB_H
#define ABB_H

#include <abb/island.h>
#include <abb/ll/successBrick.h>
#include <abb/block.h>

namespace abb {

template<typename... Args>
Block<void(Args...)> success(Args... args) {
    return std::unique_ptr<ll::Brick<void(Args...)>>(ll::SuccessBrick<void(Args...)>::newWithResult(args...));
}

} // namespace abb

#endif // ABB_H