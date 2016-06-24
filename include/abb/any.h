#ifndef ABB_ANY_H
#define ABB_ANY_H

#include <abb/ll/any_brick.h>
#include <abb/ll/bridge.h>

namespace abb {

namespace internal {

template<typename Iterator>
using any_of_return_t = typename std::decay<decltype(*std::declval<Iterator>())>::type;

using std::begin;
using std::end;

template<typename Range>
struct any_of_range_traits {};

template<typename Range>
struct any_of_range_traits<Range &> {
    typedef decltype(begin(std::declval<Range&>())) iterator;
    typedef any_of_return_t<iterator> block_type;

    static iterator do_begin(Range & range) {
        return begin(range);
    }

    static iterator do_end(Range & range) {
        return end(range);
    }
};

template<typename Range>
struct any_of_range_traits<Range &&> {
    typedef std::move_iterator<decltype(begin(std::declval<Range&>()))> iterator;
    typedef any_of_return_t<iterator> block_type;

    static iterator do_begin(Range && range) {
        return std::make_move_iterator(begin(range));
    }

    static iterator do_end(Range && range) {
        return std::make_move_iterator(end(range));
    }
};

} // namespace internal

template<typename... Results, typename... Reasons>
base_block<common_value_t<Results...>, common_value_t<Reasons...>> any(base_block<Results, Reasons> &&... blocks) {
    typedef base_block<common_value_t<Results...>, common_value_t<Reasons...>> block_type;
    typedef ll::any_brick<get_result_t<block_type>, get_reason_t<block_type>> any_brick_type;
    ll::get_brick_ptr_t<block_type> bricks[] = {ll::unpack_brick_ptr(std::move(blocks))...};

    return ll::pack_brick<any_brick_type>(std::begin(bricks), std::end(bricks));
}

template<typename Iterator>
internal::any_of_return_t<Iterator> any_of(Iterator begin, Iterator end) {
    typedef internal::any_of_return_t<Iterator> block_type;
    typedef ll::any_brick<get_result_t<block_type>, get_reason_t<block_type>> any_brick_type;

    return ll::pack_brick<any_brick_type>(ll::make_unpack_iterator(begin), ll::make_unpack_iterator(end));
}

template<typename Range>
typename internal::any_of_range_traits<Range &&>::block_type any_of(Range && range) {
    typedef internal::any_of_range_traits<Range &&> traits;
    return any_of(traits::do_begin(std::forward<Range>(range)), traits::do_end(std::forward<Range>(range)));
}

und_block hold();

} // namespace abb

#endif // ABB_ANY_H
