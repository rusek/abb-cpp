#ifndef ABB_UTILS_ALTERNATIVE_H
#define ABB_UTILS_ALTERNATIVE_H

namespace abb {
namespace utils {

template<typename ArgT, typename... ArgsT>
struct Alternative {
    typedef ArgT Type;
};

template<typename... ArgsT>
struct Alternative<void, ArgsT...> : Alternative<ArgsT...> {};

template<typename... ArgsT>
using AlternativeT = typename Alternative<ArgsT...>::Type;

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_ALTERNATIVE_H
