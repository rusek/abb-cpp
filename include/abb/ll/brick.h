#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/island.h>
#include <abb/special.h>

#include <abb/utils/noncopyable.h>

#include <tuple>

namespace abb {
namespace ll {

namespace internal {

template<typename ValueT>
struct ValueToTupleImpl {};

template<typename... ArgsT>
struct ValueToTupleImpl<void(ArgsT...)> {
    typedef std::tuple<ArgsT...> Type;
};

template<>
struct ValueToTupleImpl<Und> {
    typedef Und Type;
};

} // namespace internal

class Successor {
public:
    virtual ~Successor() {}
    virtual void oncomplete() = 0;
    virtual Island & getIsland() const = 0;
};

enum {
    PENDING = 0,
    SUCCESS = 1,
    ERROR = 2,
    ABORT = 4
};

typedef int Status;

template<typename ValueT>
using ValueToTuple = typename internal::ValueToTupleImpl<ValueT>::Type;

template<typename ResultT, typename ReasonT>
struct Brick : private utils::Noncopyable {
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
