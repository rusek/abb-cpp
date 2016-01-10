#ifndef ABB_UTILS_CALL_RESULT_H
#define ABB_UTILS_CALL_RESULT_H

namespace abb {
namespace utils {

namespace internal {

template<typename A>
A a();

} // namespace internal

template<typename Func, typename... Args>
struct CallResult {
    typedef decltype(internal::a<Func>()(internal::a<Args>()...)) Type;
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_CALL_RESULT_H
