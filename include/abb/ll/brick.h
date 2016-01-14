#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/ll/successor.h>

#include <abb/utils/noncopyable.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class Brick : utils::Noncopyable {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;
    typedef Brick<ResultT, ReasonT> BrickType;
    typedef Successor<ResultT, ReasonT> SuccessorType;

    virtual void setSuccessor(SuccessorType & successor) = 0;

    virtual ~Brick() {}
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
