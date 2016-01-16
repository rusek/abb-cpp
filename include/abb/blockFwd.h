#ifndef ABB_BLOCK_FWD
#define ABB_BLOCK_FWD

#include <abb/und.h>

#include <functional>
#include <type_traits>

namespace abb {

namespace internal {

template<typename ArgT>
struct NormalizeArg {
    typedef ArgT Type;
};

template<typename ArgT>
struct NormalizeArg<std::reference_wrapper<ArgT>> {
    typedef ArgT & Type;
};

template<typename ArgT>
using NormalizeArgT = typename NormalizeArg<ArgT>::Type;

template<typename ValueT>
struct NormalizeValue {
    typedef void Type(NormalizeArgT<ValueT>);
};

template<>
struct NormalizeValue<Und> {
    typedef Und Type;
};

template<typename ReturnT, typename... ArgsT>
struct NormalizeValue<ReturnT(ArgsT...)> {};

template<typename... ArgsT>
struct NormalizeValue<void(ArgsT...)> {
    typedef void Type(NormalizeArgT<ArgsT>...);
};

template<>
struct NormalizeValue<void> {
    typedef void Type();
};

template<typename ValueT>
using NormalizeValueT = typename NormalizeValue<ValueT>::Type;

} // namespace internal

template<typename ResultT, typename ReasonT>
class BaseBlock;

template<typename ResultT = Und, typename ReasonT = Und>
using Block = BaseBlock<internal::NormalizeValueT<ResultT>, internal::NormalizeValueT<ReasonT>>;

template<typename ReasonT>
using ErrorBlock = Block<Und, ReasonT>;

} // namespace abb

#endif // ABB_BLOCK_FWD
