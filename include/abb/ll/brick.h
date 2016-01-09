#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/ll/successor.h>

#include <abb/utils/noncopyable.h>

namespace abb {
namespace ll {

template<typename DoneCont>
class Brick {};

template<typename... Args>
class Brick<void(Args...)> : utils::Noncopyable {
public:
    virtual void setSuccessor(Successor<void(Args...)> & successor) = 0;

    virtual ~Brick() {}
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
