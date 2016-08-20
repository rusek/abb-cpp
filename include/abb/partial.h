#ifndef ABB_PARTIAL_H
#define ABB_PARTIAL_H

#include <abb/utils/integer_sequence.h>
#include <abb/utils/invoke.h>

namespace abb {

namespace internal {

template<typename Spec, typename Indices>
class partial_func;

template<typename Func, typename... Args, std::size_t... Indices>
class partial_func<Func(Args...), utils::index_sequence<Indices...>> {
private:
    Func func;
    std::tuple<Args...> args;

public:
    template<typename CtorFunc, typename... CtorArgs>
    partial_func(CtorFunc && func, CtorArgs &&... args):
        func(std::forward<CtorFunc>(func)),
        args(std::forward<CtorArgs>(args)...) {}
    partial_func(partial_func const&) = default;
    partial_func(partial_func &&) = default;

    template<typename... CallArgs>
    auto operator()(CallArgs &&... args) && -> decltype(utils::invoke(
        std::move(func),
        std::get<Indices>(std::move(this->args))...,
        std::forward<CallArgs>(args)...
    )) {
        return utils::invoke(
            std::move(func),
            std::get<Indices>(std::move(this->args))...,
            std::forward<CallArgs>(args)...
        );
    }

    template<typename... CallArgs>
    auto operator()(CallArgs &&... args) & -> decltype(utils::invoke(
        func,
        std::get<Indices>(this->args)...,
        std::forward<CallArgs>(args)...
    )) {
        return utils::invoke(
            func,
            std::get<Indices>(this->args)...,
            std::forward<CallArgs>(args)...
        );
    }

    template<typename... CallArgs>
    auto operator()(CallArgs &&... args) const& -> decltype(utils::invoke(
        func,
        std::get<Indices>(this->args)...,
        std::forward<CallArgs>(args)...
    )) {
        return utils::invoke(
            func,
            std::get<Indices>(this->args)...,
            std::forward<CallArgs>(args)...
        );
    }
};

template<typename Func, typename... Args>
using partial_return = partial_func<
    typename std::decay<Func>::type (typename std::decay<Args>::type...),
    utils::index_sequence_for<Args...>
>;

} // namespace internal

template<typename Func, typename... Args>
internal::partial_return<Func, Args...> partial(Func && func, Args &&... args) {
    return internal::partial_return<Func, Args...>(
        std::forward<Func>(func),
        std::forward<Args>(args)...
    );
}

} // namespace abb

#endif // ABB_PARTIAL_H
