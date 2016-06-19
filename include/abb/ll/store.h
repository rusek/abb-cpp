#ifndef ABB_LL_STORE_H
#define ABB_LL_STORE_H

#include <cstdint>
#include <type_traits>
#include <utility>

namespace abb {
namespace ll {

template<typename ValueT>
class Store;

template<typename ValueT, std::size_t IndexV>
class StoreGetter {};

template<typename... ArgsT, std::size_t IndexV>
class StoreGetter<void(ArgsT...), IndexV> {
private:
    typedef std::integral_constant<std::size_t, IndexV> IndexT;

public:
    auto operator()(Store<void(ArgsT...)> && store) -> decltype(std::move(store).get(IndexT())) {
        return std::move(store).get(IndexT());
    }
};

template<typename... GettersT>
struct StoreGetters {};

namespace internal {

template<typename ValueT, std::size_t SizeV, typename... GettersT>
struct GenStoreGetters : GenStoreGetters<ValueT, SizeV - 1, StoreGetter<ValueT, SizeV - 1>, GettersT...> {};

template<typename ValueT, typename... GettersT>
struct GenStoreGetters<ValueT, 0, GettersT...> {
    typedef StoreGetters<GettersT...> Type;
};

template<typename ValueT>
struct GetStoreGettersImpl {};

template<typename... ArgsT>
struct GetStoreGettersImpl<void(ArgsT...)> {
    typedef typename internal::GenStoreGetters<void(ArgsT...), sizeof...(ArgsT)>::Type Type;
};

template<std::size_t IndexV, typename... ArgsT>
class StoreImpl {};

template<typename std::size_t IndexV, typename ArgT>
class StoreImpl<IndexV, ArgT> {
public:
    explicit StoreImpl(ArgT const& arg) : arg(arg) {}
    explicit StoreImpl(ArgT && arg) : arg(std::move(arg)) {}

    ArgT && get(std::integral_constant<std::size_t, IndexV>) && {
        return std::move(this->arg);
    }

private:
    ArgT arg;
};

template<typename std::size_t IndexV, typename ArgT>
class StoreImpl<IndexV, ArgT &> {
public:
    template<typename... CtorArgsT>
    explicit StoreImpl(ArgT & arg) : arg(std::addressof(arg)) {}

    ArgT & get(std::integral_constant<std::size_t, IndexV>) && {
        return *this->arg;
    }

private:
    ArgT * arg;
};

template<typename std::size_t IndexV, typename ArgT, typename... ArgsT>
class StoreImpl<IndexV, ArgT, ArgsT...> : public StoreImpl<IndexV, ArgT>, StoreImpl<IndexV + 1, ArgsT...> {
public:
    using StoreImpl<IndexV, ArgT>::get;
    using StoreImpl<IndexV + 1, ArgsT...>::get;

    template<typename CtorArgT, typename... CtorArgsT>
    explicit StoreImpl(CtorArgT && arg, CtorArgsT &&... args):
        StoreImpl<IndexV, ArgT>(std::forward<CtorArgT>(arg)),
        StoreImpl<IndexV + 1, ArgsT...>(std::forward<CtorArgsT>(args)...) {}
};

} // namespace internal

template<typename ValueT>
using GetStoreGetters = typename internal::GetStoreGettersImpl<ValueT>::Type;

template<typename ValueT>
class Store {};

template<typename... ArgsT>
class Store<void(ArgsT...)> : private internal::StoreImpl<0, ArgsT...> {
public:
    template<typename... CtorArgsT>
    explicit Store(CtorArgsT &&... args): internal::StoreImpl<0, ArgsT...>(std::forward<CtorArgsT>(args)...) {}

private:
    template<typename FriendValueT, std::size_t FriendIndexV>
    friend class StoreGetter;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_STORE_H
