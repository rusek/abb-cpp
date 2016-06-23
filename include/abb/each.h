#ifndef ABB_EACH_H
#define ABB_EACH_H

#include <abb/ll/eachBrick.h>
#include <abb/ll/bridge.h>
#include <abb/utils/range.h>

#include <iterator>
#include <limits>

namespace abb {

namespace internal {

template<typename IteratorT, typename FuncT>
using EachReturn = decltype(std::declval<FuncT>()(*std::declval<IteratorT>()));

template<typename RangeT, typename FuncT>
using EachRangeReturn = decltype(std::declval<FuncT>()(*abb::utils::begin(std::declval<RangeT&>())));

} // namespace internal

template<typename IteratorT, typename FuncT>
internal::EachReturn<IteratorT, FuncT> each(
    IteratorT begin,
    IteratorT end,
    FuncT && func,
    std::size_t limit = std::numeric_limits<std::size_t>::max()
) {
    typedef ll::EachBrick<
        ll::MapGenerator<
            ll::IteratorGenerator<IteratorT>,
            ll::Unpacker<typename std::decay<FuncT>::type>
        >
    > EachBrickType;
    return ll::packBrick<EachBrickType>(
        inplace(
            inplace(begin, end),
            inplace(std::forward<FuncT>(func))
        ),
        limit
    );
}

template<typename RangeT, typename FuncT>
internal::EachRangeReturn<RangeT, FuncT> each(
    RangeT && range,
    FuncT && func,
    std::size_t limit = std::numeric_limits<std::size_t>::max()
) {
    typedef ll::EachBrick<
        ll::MapGenerator<
            ll::RangeGenerator<typename std::decay<RangeT>::type>,
            ll::Unpacker<typename std::decay<FuncT>::type>
        >
    > EachBrickType;
    return ll::packBrick<EachBrickType>(
        inplace(
            inplace(std::forward<RangeT>(range)),
            inplace(std::forward<FuncT>(func))
        ),
        limit
    );
}

} // namespace abb

#endif // ABB_EACH_H
