#ifndef ABB_INPLACE_H
#define ABB_INPLACE_H

#include <tuple>

namespace abb {

template<typename... Items>
class inplace_tuple : private std::tuple<typename Items::type...> {
public:
    using std::tuple<typename Items::type...>::tuple;

    template<typename Arg, std::size_t Index>
    friend struct inplace_item;
};

template<typename Arg, std::size_t Index>
struct inplace_item {
    typedef Arg type;
    static std::size_t const index = Index;

    template<typename... Items>
    static Arg get(inplace_tuple<Items...> const& inplace) {
        // we use tuple under the hood, and to preserve element type unchanged, we need
        // to pass tuple as rvalue to std::get
        return std::get<Index>(std::move(const_cast<inplace_tuple<Items...> &>(inplace)));
    }
};

namespace internal {

template<typename Acc, std::size_t Index, typename... Args>
struct inplace_return {
    typedef Acc type;
};

template<typename... Items, std::size_t Index, typename Arg, typename... Args>
struct inplace_return<inplace_tuple<Items...>, Index, Arg, Args...> :
    inplace_return<inplace_tuple<Items..., inplace_item<Arg, Index>>, Index + 1, Args...> {};

template<typename... Args>
using inplace_return_t = typename inplace_return<inplace_tuple<>, 0, Args...>::type;

} // namespace internal

template<typename... Args>
internal::inplace_return_t<Args &&...> inplace(Args &&... args) {
    return internal::inplace_return_t<Args &&...>(std::forward<Args>(args)...);
}

} // namespace abb

#endif // ABB_INPLACE_H
