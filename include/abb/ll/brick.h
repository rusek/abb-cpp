#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/island.h>
#include <abb/value.h>

#include <abb/utils/noncopyable.h>

#include <tuple>

namespace abb {
namespace ll {

namespace internal {

struct BrickVtable;

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

class RawBrick : private utils::Noncopyable {
public:
    internal::BrickVtable const* vtable;
};

template<typename FriendBrickT>
class BrickFuncs;

} // namespace internal

class Successor {
public:
    virtual ~Successor() {}
    virtual void onUpdate() = 0;
    virtual Island & getIsland() const = 0;
    virtual bool isAborted() const = 0;
};

enum {
    PENDING = 0,
    SUCCESS = 1,
    ERROR = 2,
    ABORT = 4,
    NEXT = 8
};

typedef int Status;

template<typename ValueT>
using ValueToTuple = typename internal::ValueToTupleImpl<ValueT>::Type;

template<typename ResultT, typename ReasonT>
class BrickPtr;

template<typename ResultT, typename ReasonT>
struct Brick : private internal::RawBrick {
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    template<typename FriendBrickT>
    friend class internal::BrickFuncs;

    BrickPtr<ResultT, ReasonT> getNext() {
        ABB_FIASCO("Erased method called");
    }

    ValueToTuple<ResultT> & getResult() {
        ABB_FIASCO("Erased method called");
    }

    ValueToTuple<ReasonT> & getReason() {
        ABB_FIASCO("Erased method called");
    }
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
