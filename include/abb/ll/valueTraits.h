#ifndef ABB_LL_VALUE_TRAITS
#define ABB_LL_VALUE_TRAITS

#include <functional>
#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename ArgT>
struct UnwrapRef {
    typedef ArgT Type;
};

template<typename ArgT>
struct UnwrapRef<std::reference_wrapper<ArgT>> {
    typedef ArgT & Type;
};

} // namespace internal

template<typename... ArgsT>
struct ArgsToValue {
    typedef void Type(typename internal::UnwrapRef<typename std::decay<ArgsT>::type>::Type...);
};

}; // namespace ll
}; // namespace abb

#endif // ABB_LL_VALUE_TRAITS
