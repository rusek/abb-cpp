#ifndef ABB_BLOCK_FWD
#define ABB_BLOCK_FWD

#include <abb/special.h>

#include <functional>
#include <type_traits>

namespace abb {

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

template<typename ValueT>
using NormalizeValue = typename NormalizeValueImpl<ValueT>::Type;

} // namespace internal

template<typename ResultT, typename ReasonT>
class BaseBlock;

template<typename ResultT = Und, typename ReasonT = Und>
using Block = BaseBlock<internal::NormalizeValue<ResultT>, internal::NormalizeValue<ReasonT>>;

} // namespace abb

#endif // ABB_BLOCK_FWD
