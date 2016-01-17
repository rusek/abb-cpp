#ifndef ABB_UTILS_CALL_H
#define ABB_UTILS_CALL_H

#include <tuple>
#include <type_traits>

namespace abb {
namespace utils {

namespace internal {

template <typename Func, typename Tuple, std::size_t Rem, std::size_t... I>
struct Call : Call<Func, Tuple, Rem - 1, Rem - 1, I...> {};

template <typename Func, typename Tuple, std::size_t... I>
struct Call<Func, Tuple, 0, I...> {
    static auto call(Func func, Tuple && tuple) -> decltype(func(std::get<I>(std::forward<Tuple>(tuple))...)) {
        return func(std::get<I>(std::forward<Tuple>(tuple))...);
    }
};

template<typename Func, typename Tuple>
using CallR = Call<Func, Tuple, std::tuple_size<typename std::decay<Tuple>::type>::value>;

} // namespace internal

template<typename Func, typename Tuple>
auto call(Func && f, Tuple && t) -> decltype(internal::CallR<Func, Tuple>::call(f, std::forward<Tuple>(t))) {
    return internal::CallR<Func, Tuple>::call(std::forward<Func>(f), std::forward<Tuple>(t));
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_CALL_H
