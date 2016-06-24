#ifndef ABB_EACH_H
#define ABB_EACH_H

#include <abb/ll/each_brick.h>
#include <abb/ll/bridge.h>
#include <abb/utils/range.h>

#include <iterator>
#include <limits>

namespace abb {

namespace internal {

template<typename Iterator, typename Func>
using each_return_t = decltype(std::declval<Func>()(*std::declval<Iterator>()));

template<typename Range, typename Func>
using each_range_return_t = decltype(std::declval<Func>()(*abb::utils::begin(std::declval<Range&>())));

} // namespace internal

template<typename Iterator, typename Func>
internal::each_return_t<Iterator, Func> each(
    Iterator begin,
    Iterator end,
    Func && func,
    std::size_t limit = std::numeric_limits<std::size_t>::max()
) {
    typedef ll::each_brick<
        ll::map_generator<
            ll::iterator_generator<Iterator>,
            ll::unpacker<typename std::decay<Func>::type>
        >
    > each_brick_type;
    return ll::pack_brick<each_brick_type>(
        inplace(
            inplace(begin, end),
            inplace(std::forward<Func>(func))
        ),
        limit
    );
}

template<typename Range, typename Func>
internal::each_range_return_t<Range, Func> each(
    Range && range,
    Func && func,
    std::size_t limit = std::numeric_limits<std::size_t>::max()
) {
    typedef ll::each_brick<
        ll::map_generator<
            ll::range_generator<typename std::decay<Range>::type>,
            ll::unpacker<typename std::decay<Func>::type>
        >
    > each_brick_type;
    return ll::pack_brick<each_brick_type>(
        inplace(
            inplace(std::forward<Range>(range)),
            inplace(std::forward<Func>(func))
        ),
        limit
    );
}

} // namespace abb

#endif // ABB_EACH_H
