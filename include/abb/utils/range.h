#ifndef ABB_UTILS_RANGE_H
#define ABB_UTILS_RANGE_H

#include <iterator>

namespace abb_adl {

using std::begin;
using std::end;

template<typename ArgT>
inline auto doBegin(ArgT && arg) -> decltype(begin(std::forward<ArgT>(arg))) {
    return begin(std::forward<ArgT>(arg));
}

template<typename ArgT>
inline auto doEnd(ArgT && arg) -> decltype(end(std::forward<ArgT>(arg))) {
    return end(std::forward<ArgT>(arg));
}

} // namespace abb_adl

namespace abb {
namespace utils {

template<typename ArgT>
inline auto begin(ArgT && arg) -> decltype(abb_adl::doBegin(std::forward<ArgT>(arg))) {
    return abb_adl::doBegin(std::forward<ArgT>(arg));
}

template<typename ArgT>
inline auto end(ArgT && arg) -> decltype(abb_adl::doEnd(std::forward<ArgT>(arg))) {
    return abb_adl::doEnd(std::forward<ArgT>(arg));
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_RANGE_H
