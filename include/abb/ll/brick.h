#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/ll/successor.h>

#include <abb/utils/noncopyable.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class Brick : utils::Noncopyable {
public:
    typedef Brick<Result, Reason> BaseType;
    typedef Successor<Result, Reason> SuccessorType;

    virtual void setSuccessor(SuccessorType & successor) = 0;

    virtual ~Brick() {}
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
