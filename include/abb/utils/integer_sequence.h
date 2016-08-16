#ifndef ABB_UTILS_INTEGER_SEQUENCE_H
#define ABB_UTILS_INTEGER_SEQUENCE_H

#include <cstddef>

namespace abb {
namespace utils {

template<typename Int, Int... Elems>
struct integer_sequence {};

template<std::size_t... Elems>
using index_sequence = integer_sequence<std::size_t, Elems...>;

namespace internal {

template<std::size_t Rem, std::size_t... Elems>
struct index_sequence_maker : index_sequence_maker<Rem - 1, Rem - 1, Elems...> {};

template<typename std::size_t... Elems>
struct index_sequence_maker<0, Elems...> {
    typedef index_sequence<Elems...> type;
};

};

template<typename std::size_t Size>
using make_index_sequence = typename internal::index_sequence_maker<Size>::type;

template<typename... Types>
using index_sequence_for = make_index_sequence<sizeof...(Types)>;

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_INTEGER_SEQUENCE_H
