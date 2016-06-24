#ifndef ABB_LL_STORE_H
#define ABB_LL_STORE_H

#include <cstdint>
#include <type_traits>
#include <utility>

namespace abb {
namespace ll {

template<typename Value>
class store;

template<typename Value, std::size_t Index>
class store_getter {};

template<typename... Args, std::size_t Index>
class store_getter<void(Args...), Index> {
private:
    typedef std::integral_constant<std::size_t, Index> index_constant;

public:
    auto operator()(store<void(Args...)> && store) -> decltype(std::move(store).get(index_constant())) {
        return std::move(store).get(index_constant());
    }
};

template<typename... Getters>
struct store_getters {};

namespace internal {

template<typename Value, std::size_t Size, typename... Getters>
struct gen_store_getters : gen_store_getters<Value, Size - 1, store_getter<Value, Size - 1>, Getters...> {};

template<typename Value, typename... Getters>
struct gen_store_getters<Value, 0, Getters...> {
    typedef store_getters<Getters...> type;
};

template<typename Value>
struct get_store_getters {};

template<typename... Args>
struct get_store_getters<void(Args...)> {
    typedef typename internal::gen_store_getters<void(Args...), sizeof...(Args)>::type type;
};

template<std::size_t Index, typename... Args>
class store_segment {};

template<typename std::size_t Index, typename Arg>
class store_segment<Index, Arg> {
public:
    explicit store_segment(Arg const& arg) : arg(arg) {}
    explicit store_segment(Arg && arg) : arg(std::move(arg)) {}

    Arg && get(std::integral_constant<std::size_t, Index>) && {
        return std::move(this->arg);
    }

private:
    Arg arg;
};

template<typename std::size_t Index, typename Arg>
class store_segment<Index, Arg &> {
public:
    template<typename... CtorArgs>
    explicit store_segment(Arg & arg) : arg(std::addressof(arg)) {}

    Arg & get(std::integral_constant<std::size_t, Index>) && {
        return *this->arg;
    }

private:
    Arg * arg;
};

template<typename std::size_t Index, typename Arg, typename... Args>
class store_segment<Index, Arg, Args...> : public store_segment<Index, Arg>, store_segment<Index + 1, Args...> {
public:
    using store_segment<Index, Arg>::get;
    using store_segment<Index + 1, Args...>::get;

    template<typename CtorArg, typename... CtorArgs>
    explicit store_segment(CtorArg && arg, CtorArgs &&... args):
        store_segment<Index, Arg>(std::forward<CtorArg>(arg)),
        store_segment<Index + 1, Args...>(std::forward<CtorArgs>(args)...) {}
};

} // namespace internal

template<typename Value>
using get_store_getters_t = typename internal::get_store_getters<Value>::type;

template<typename Value>
class store {};

template<typename... Args>
class store<void(Args...)> : private internal::store_segment<0, Args...> {
public:
    template<typename... CtorArgs>
    explicit store(CtorArgs &&... args): internal::store_segment<0, Args...>(std::forward<CtorArgs>(args)...) {}

private:
    template<typename FriendValue, std::size_t FriendIndex>
    friend class store_getter;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_STORE_H
