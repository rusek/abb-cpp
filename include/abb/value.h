#ifndef ABB_VALUE_H
#define ABB_VALUE_H

#include <type_traits>

namespace abb {

class Und {};

class Pass {};

namespace internal {

template<typename ArgT>
struct NormalizeArgImpl {
    typedef ArgT Type;
};

template<typename ArgT>
struct NormalizeArgImpl<std::reference_wrapper<ArgT>> {
    typedef ArgT & Type;
};

template<typename ArgT>
using NormalizeArg = typename NormalizeArgImpl<ArgT>::Type;

template<typename ValueT>
struct NormalizeValueImpl {
    typedef void Type(NormalizeArg<ValueT>);
};

template<>
struct NormalizeValueImpl<Und> {
    typedef Und Type;
};

template<typename ReturnT, typename... ArgsT>
struct NormalizeValueImpl<ReturnT(ArgsT...)> {};

template<typename... ArgsT>
struct NormalizeValueImpl<void(ArgsT...)> {
    typedef void Type(NormalizeArg<ArgsT>...);
};

template<>
struct NormalizeValueImpl<void> {
    typedef void Type();
};

} // namespace internal

const Und und;

const Pass pass;

template<typename ValueT>
struct IsUnd : std::is_same<ValueT, Und> {};

template<typename ValueT>
struct IsPass : std::is_same<ValueT, Pass> {};

template<typename ValueT>
struct IsSpecial : std::integral_constant<bool, IsUnd<ValueT>::value || IsPass<ValueT>::value> {};

template<typename ArgT>
using GetResult = typename ArgT::ResultType;

template<typename ArgT>
using GetReason = typename ArgT::ReasonType;

template<typename ValueT>
using NormalizeValue = typename internal::NormalizeValueImpl<ValueT>::Type;

template<typename ValueT, typename OtherValueT>
struct IsValueSubstitutable : std::integral_constant<
    bool,
    std::is_same<ValueT, OtherValueT>::value ||
        std::is_same<OtherValueT, Und>::value
> {};

namespace internal {

template<typename... TypesT>
struct CommonValueImpl {};

template<>
struct CommonValueImpl<> {
    typedef Und Type;
};

template<typename TypeT>
struct CommonValueImpl<TypeT> {
    typedef TypeT Type;
};

template<typename FirstT, typename SecondT>
struct CommonValueImpl<FirstT, SecondT> {
    typedef typename std::conditional<IsUnd<FirstT>::value, SecondT, FirstT>::type Type;
    static_assert(IsValueSubstitutable<Type, SecondT>::value, "Incompatible types");
};

template<typename FirstT, typename SecondT, typename... TypesT>
struct CommonValueImpl<FirstT, SecondT, TypesT...> :
    CommonValueImpl<typename CommonValueImpl<FirstT, SecondT>::Type, TypesT...> {};

} // namespace internal

template<typename... TypesT>
using CommonValue = typename internal::CommonValueImpl<TypesT...>::Type;

} // namespace abb

#endif // ABB_VALUE_H
