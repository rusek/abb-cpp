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
    auto operator()(Store<void(ArgsT...)> && store) -> decltype(store.move(IndexT())) {
        return store.move(IndexT());
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

template<typename std::size_t IndexV, typename ArgT>
class StoreItem {
public:
    explicit StoreItem(ArgT const& arg) : arg(arg) {}
    explicit StoreItem(ArgT && arg) : arg(std::move(arg)) {}

    ArgT && move(std::integral_constant<std::size_t, IndexV>) {
        return std::move(this->arg);
    }

private:
    ArgT arg;
};

template<typename std::size_t IndexV, typename ArgT>
class StoreItem<IndexV, ArgT &> {
public:
    template<typename... CtorArgsT>
    explicit StoreItem(ArgT & arg) : arg(std::addressof(arg)) {}

    ArgT & move(std::integral_constant<std::size_t, IndexV>) {
        return *this->arg;
    }

private:
    ArgT * arg;
};

template<std::size_t IndexV, typename... ArgsT>
class StoreTail {};

template<typename std::size_t IndexV, typename ArgT, typename... ArgsT>
class StoreTail<IndexV, ArgT, ArgsT...> : public StoreItem<IndexV, ArgT>, StoreTail<IndexV + 1, ArgsT...> {
public:
    template<typename CtorArgT, typename... CtorArgsT>
    explicit StoreTail(CtorArgT && arg, CtorArgsT &&... args):
        StoreItem<IndexV, ArgT>(std::forward<CtorArgT>(arg)),
        StoreTail<IndexV + 1, ArgsT...>(std::forward<CtorArgsT>(args)...) {}
};

} // namespace internal

template<typename ValueT>
using GetStoreGetters = typename internal::GetStoreGettersImpl<ValueT>::Type;

template<typename ValueT>
class Store {};

template<typename... ArgsT>
class Store<void(ArgsT...)> : private internal::StoreTail<0, ArgsT...> {
public:
    template<typename... CtorArgsT>
    explicit Store(CtorArgsT &&... args): internal::StoreTail<0, ArgsT...>(std::forward<CtorArgsT>(args)...) {}

private:
    template<typename FriendValueT, std::size_t FriendIndexV>
    friend class StoreGetter;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_STORE_H
