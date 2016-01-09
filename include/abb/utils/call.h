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
    static void call(Func func, Tuple && tuple) {
        func(std::get<I>(std::forward<Tuple>(tuple))...);
    }
};

} // namespace internal

template<typename Func, typename Tuple>
void call(Func f, Tuple && t) {
    typedef typename std::decay<Tuple>::type DTuple;
    internal::Call<Func, Tuple, std::tuple_size<DTuple>::value>::call(f, std::forward<Tuple>(t));
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_CALL_H
