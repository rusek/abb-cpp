#ifndef ABB_UTILS_CALL_RESULT_H
#define ABB_UTILS_CALL_RESULT_H

namespace abb {
namespace utils {

namespace internal {

template<typename ReturnT>
ReturnT a();

} // namespace internal

template<typename FuncT, typename... ArgsT>
struct CallResult {
    typedef decltype(internal::a<FuncT>()(internal::a<ArgsT>()...)) Type;
};

template<typename FuncT, typename... ArgsT>
using CallReturn = typename CallResult<FuncT, ArgsT...>::Type;

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_CALL_RESULT_H
