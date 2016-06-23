#ifndef ABB_INPLACE_H
#define ABB_INPLACE_H

#include <tuple>

namespace abb {

template<typename... ItemsT>
class Inplace : private std::tuple<typename ItemsT::Type...> {
public:
    using std::tuple<typename ItemsT::Type...>::tuple;

    template<typename ArgT, std::size_t IndexV>
    friend struct InplaceItem;
};

template<typename ArgT, std::size_t IndexV>
struct InplaceItem {
    typedef ArgT Type;
    static std::size_t const index = IndexV;

    template<typename... ItemsT>
    static ArgT get(Inplace<ItemsT...> const& inplace) {
        // we use tuple under the hood, and to preserve element type unchanged, we need
        // to pass tuple as rvalue to std::get
        return std::get<IndexV>(std::move(const_cast<Inplace<ItemsT...> &>(inplace)));
    }
};

namespace internal {

template<typename AccT, std::size_t IndexV, typename... ArgsT>
struct InplaceReturnImpl {
    typedef AccT Type;
};

template<typename... ItemsT, std::size_t IndexV, typename ArgT, typename... ArgsT>
struct InplaceReturnImpl<Inplace<ItemsT...>, IndexV, ArgT, ArgsT...> :
    InplaceReturnImpl<Inplace<ItemsT..., InplaceItem<ArgT, IndexV>>, IndexV + 1, ArgsT...> {};

template<typename... ArgsT>
using InplaceReturn = typename InplaceReturnImpl<Inplace<>, 0, ArgsT...>::Type;

} // namespace internal

template<typename... ArgsT>
internal::InplaceReturn<ArgsT &&...> inplace(ArgsT &&... args) {
    return internal::InplaceReturn<ArgsT &&...>(std::forward<ArgsT>(args)...);
}

} // namespace abb

#endif // ABB_INPLACE_H
