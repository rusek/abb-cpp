#ifndef ABB_LL_BRICK_H
#define ABB_LL_BRICK_H

#include <abb/ll/successor.h>

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

template<typename ValueT>
using ValueToTuple = typename internal::ValueToTupleImpl<ValueT>::Type;

template<typename ResultT, typename ReasonT>
struct Brick : Brick<ResultT, Und>, Brick<Und, ReasonT> {
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    virtual void setSuccessor(Successor & successor) = 0;
};

template<>
struct Brick<Und, Und> : private utils::Noncopyable {
    typedef Und ResultType;
    typedef Und ReasonType;

    virtual void setSuccessor(Successor & successor) = 0;
};

template<typename ResultT>
struct Brick<ResultT, Und> : Brick<Und, Und> {
    typedef ResultT ResultType;
    typedef Und ReasonType;

    virtual bool hasResult() const = 0;
    virtual ValueToTuple<ResultType> & getResult() = 0;
};

template<typename ReasonT>
struct Brick<Und, ReasonT> : Brick<Und, Und> {
    typedef Und ResultType;
    typedef ReasonT ReasonType;;

    virtual bool hasReason() const = 0;
    virtual ValueToTuple<ReasonType> & getReason() = 0;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_H
