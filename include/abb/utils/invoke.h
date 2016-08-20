#ifndef ABB_UTILS_INVOKE_H
#define ABB_UTILS_INVOKE_H

#include <type_traits>
#include <functional>

namespace abb {
namespace utils {

namespace internal {

template<typename Type>
struct is_reference_wrapper : std::false_type {};

template<typename Type>
struct is_reference_wrapper<std::reference_wrapper<Type>> : std::true_type {};

} // namespace internal

template<typename Base, typename Obj>
inline typename std::enable_if<
    std::is_base_of<Base, typename std::decay<Obj>::type>::value,
    Obj &&
>::type deref(Obj && obj) {
    return std::forward<Obj>(obj);
}

template<typename Base, typename Obj>
inline auto deref(Obj && obj) -> typename std::enable_if<
    !std::is_base_of<Base, typename std::decay<Obj>::type>::value &&
        internal::is_reference_wrapper<typename std::decay<Obj>::type>::value,
    decltype(obj.get())
>::type {
    return obj.get();
}

template<typename Base, typename Obj>
inline auto deref(Obj && obj) -> typename std::enable_if<
    !std::is_base_of<Base, typename std::decay<Obj>::type>::value &&
        !internal::is_reference_wrapper<typename std::decay<Obj>::type>::value,
    decltype(*std::forward<Obj>(obj))
>::type {
    return *std::forward<Obj>(obj);
}

namespace internal {

template<typename Type>
struct is_function : std::is_function<Type> {};

template<typename Ret, typename... Args>
struct is_function<Ret(Args...) &&> : std::true_type {};

} // namespace internal

template<typename Func, typename... Args>
inline auto invoke(Func && func, Args &&... args) -> decltype(std::forward<Func>(func)(std::forward<Args>(args)...)) {
    return std::forward<Func>(func)(std::forward<Args>(args)...);
}

template<typename Func, typename Base, typename Obj, typename... Args>
inline auto invoke(Func Base::* func, Obj && obj, Args &&... args) -> typename std::enable_if<
    internal::is_function<Func>::value,
    decltype((deref<Base>(std::forward<Obj>(obj)).*func)(std::forward<Args>(args)...))
>::type {
    return (deref<Base>(std::forward<Obj>(obj)).*func)(std::forward<Args>(args)...);
}

template<typename Prop, typename Base, typename Obj, typename... Args>
inline auto invoke(Prop Base::* prop, Obj && obj, Args &&... args) -> typename std::enable_if<
    !internal::is_function<Prop>::value,
    decltype(deref<Base>(std::forward<Obj>(obj)).*prop)
>::type {
    return deref<Base>(std::forward<Obj>(obj)).*prop;
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_INVOKE_H
