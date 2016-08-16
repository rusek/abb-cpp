#ifndef ABB_VALUE_H
#define ABB_VALUE_H

#include <type_traits>
#include <tuple>

namespace abb {

class und_t {};

class pass_t {};

const und_t und;

const pass_t pass;

template<typename Value>
struct is_und : std::is_same<Value, und_t> {};

template<typename Value>
struct is_pass : std::is_same<Value, pass_t> {};

template<typename Value>
struct is_special : std::integral_constant<bool, is_und<Value>::value || is_pass<Value>::value> {};

template<typename T>
class inplace_args;

template<typename Ret, typename... Args>
class inplace_args<Ret(Args...)> : private std::tuple<Args...> {
public:
    using std::tuple<Args...>::tuple;

    template<std::size_t Index>
    typename std::tuple_element<Index, std::tuple<Args...>>::type const& get() const {
        return std::get<Index>(*this);
    }
};

template<typename Ret = pass_t, typename... Args>
inplace_args<Ret(Args &&...)> inplace(Args &&... args) {
    return inplace_args<Ret(Args &&...)>(std::forward<Args>(args)...);
}

namespace internal {

template<typename Arg>
struct normalize_arg {
    typedef Arg type;
};

template<typename Ret, typename... Args>
struct normalize_arg<inplace_args<Ret(Args...)>> {
    static_assert(!is_special<Ret>::value, "Expected inplace_args to contain valid object type");

    typedef Ret type;
};

template<typename Arg>
struct normalize_arg<std::reference_wrapper<Arg>> {
    typedef Arg & type;
};

template<typename Arg>
using normalize_arg_t = typename normalize_arg<Arg>::type;

template<typename Value>
struct normalize_value {
    typedef void type(normalize_arg_t<Value>);
};

template<>
struct normalize_value<und_t> {
    typedef und_t type;
};

template<typename Return, typename... Args>
struct normalize_value<Return(Args...)> {};

template<typename... Args>
struct normalize_value<void(Args...)> {
    typedef void type(normalize_arg_t<Args>...);
};

template<>
struct normalize_value<void> {
    typedef void type();
};

} // namespace internal

template<typename Arg>
using get_result_t = typename Arg::result;

template<typename Arg>
using get_reason_t = typename Arg::reason;

template<typename Value>
using normalize_value_t = typename internal::normalize_value<Value>::type;

template<typename Value, typename OtherValue>
struct is_value_substitutable : std::integral_constant<
    bool,
    std::is_same<Value, OtherValue>::value ||
        std::is_same<OtherValue, und_t>::value
> {};

namespace internal {

template<typename... Types>
struct common_value {};

template<>
struct common_value<> {
    typedef und_t type;
};

template<typename Type>
struct common_value<Type> {
    typedef Type type;
};

template<typename First, typename Second>
struct common_value<First, Second> {
    typedef typename std::conditional<is_und<First>::value, Second, First>::type type;
    static_assert(is_value_substitutable<type, Second>::value, "Incompatible types");
};

template<typename First, typename Second, typename... Types>
struct common_value<First, Second, Types...> :
    common_value<typename common_value<First, Second>::type, Types...> {};

} // namespace internal

template<typename... Types>
using common_value_t = typename internal::common_value<Types...>::type;

} // namespace abb

#endif // ABB_VALUE_H
