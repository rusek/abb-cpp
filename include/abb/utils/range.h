#ifndef ABB_UTILS_RANGE_H
#define ABB_UTILS_RANGE_H

#include <iterator>

namespace abb_adl {

using std::begin;
using std::end;

template<typename Arg>
inline auto do_begin(Arg && arg) -> decltype(begin(std::forward<Arg>(arg))) {
    return begin(std::forward<Arg>(arg));
}

template<typename Arg>
inline auto do_end(Arg && arg) -> decltype(end(std::forward<Arg>(arg))) {
    return end(std::forward<Arg>(arg));
}

} // namespace abb_adl

namespace abb {
namespace utils {

template<typename Arg>
inline auto begin(Arg && arg) -> decltype(abb_adl::do_begin(std::forward<Arg>(arg))) {
    return abb_adl::do_begin(std::forward<Arg>(arg));
}

template<typename Arg>
inline auto end(Arg && arg) -> decltype(abb_adl::do_end(std::forward<Arg>(arg))) {
    return abb_adl::do_end(std::forward<Arg>(arg));
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_RANGE_H
