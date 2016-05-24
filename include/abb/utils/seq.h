#ifndef ABB_UTILS_SEQ_H
#define ABB_UTILS_SEQ_H

#include <cstdint>

namespace abb {
namespace utils {

template<std::size_t... IndicesV>
struct Seq {};

template<std::size_t SizeV, std::size_t... IndicesV>
struct GetSeq : GetSeq<SizeV - 1, SizeV - 1, IndicesV...> {};

template<std::size_t... IndicesV>
struct GetSeq<0, IndicesV...> : Seq<IndicesV...> {};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_SEQ_H
