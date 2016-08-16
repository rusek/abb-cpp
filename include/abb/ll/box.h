#ifndef ABB_LL_BOX_H
#define ABB_LL_BOX_H

#include <abb/utils/integer_sequence.h>
#include <abb/value.h>

namespace abb {
namespace ll {

namespace internal {

template<typename Ret, typename Type>
struct enable_inplace : std::enable_if<std::is_same<Ret, Type>::value || is_pass<Ret>::value> {};

} // namespace internal

struct box_arg_t {};

const box_arg_t box_arg;

template<typename Type>
class box {
public:
    box(box_arg_t, Type const& arg) : arg(arg) {}
    box(box_arg_t, Type && arg) : arg(std::move(arg)) {}
    box(box const&) = default;
    box(box &&) = default;

    template<typename Ret, typename... Args, typename internal::enable_inplace<Ret, Type>::type* = nullptr>
    box(box_arg_t, inplace_args<Ret(Args...)> const& args):
        box(args, utils::index_sequence_for<Args...>()) {}

    box & operator=(box const&) = default;
    box & operator=(box &&) = default;

    Type const& operator*() const& { return this->arg; }
    Type & operator*() & { return this->arg; }
    Type && operator*() && { return std::move(this->arg); }

    Type const* operator->() const { return std::addressof(this->arg); }
    Type * operator->() { return std::addressof(this->arg); }

private:
    template<typename Ret, typename... Args, std::size_t... Indices>
    box(inplace_args<Ret(Args...)> const& args, utils::index_sequence<Indices...>):
        arg(std::forward<Args>(args.template get<Indices>())...) {}

    Type arg;
};

template<typename Type>
class box<Type &> {
public:
    box(box_arg_t, Type & arg) : arg(prep(arg)) {}
    box(box const&) = default;

    template<typename Ret, typename Arg, typename internal::enable_inplace<Ret, Type>::type* = nullptr>
    box(box_arg_t, inplace_args<Ret(Arg)> const& args):
        arg(prep(std::forward<Arg>(args.template get<0>()))) {}

    box & operator=(box const& other) = default;

    Type & operator*() const { return *this->arg; }

    Type * operator->() const { return *this->arg; }

private:
    static Type * prep(Type & arg) { return std::addressof(arg); }

    Type * arg;
};

template<typename Type>
class box<Type &&> {
public:
    box(box_arg_t, Type && arg) : arg(prep(arg)) {}
    box(box const&) = default;

    template<typename Ret, typename Arg, typename internal::enable_inplace<Ret, Type>::type* = nullptr>
    box(box_arg_t, inplace_args<Ret(Arg)> const& args):
        arg(prep(std::forward<Arg>(args.template get<0>()))) {}


    box & operator=(box const& other) = default;

    Type & operator*() & { return *this->arg; }
    Type & operator*() const& { return *this->arg; }
    Type && operator*() && { return std::move(*this->arg); }

    Type * operator->() const { return this->arg; }

private:
    static Type * prep(Type & arg) { return std::addressof(arg); }

    Type * arg;
};

namespace internal {

template<std::size_t Index, typename... Elems>
class boxes_impl;

template<std::size_t Index>
class boxes_impl<Index> {
public:
    explicit boxes_impl(box_arg_t) {}
};

template<std::size_t Index, typename Elem>
class boxes_impl<Index, Elem> : public box<Elem> {
public:
    using box<Elem>::box;
};

template<std::size_t Index, typename Elem, typename... Elems>
class boxes_impl<Index, Elem, Elems...> : public boxes_impl<Index, Elem>, public boxes_impl<Index + 1, Elems...> {
public:
    template<typename Arg, typename... Args>
    boxes_impl(box_arg_t, Arg && arg, Args &&... args):
        boxes_impl<Index, Elem>(box_arg, std::forward<Arg>(arg)),
        boxes_impl<Index + 1, Elems...>(box_arg, std::forward<Args>(args)...) {}
};

template<std::size_t Index, typename Elem>
inline boxes_impl<Index, Elem> const& select(boxes_impl<Index, Elem> const& b) { return b; }

template<std::size_t Index, typename Elem>
inline boxes_impl<Index, Elem> & select(boxes_impl<Index, Elem> & b) { return b; }

template<std::size_t Index, typename... Elems>
struct nth;

template<typename Elem, typename... Elems>
struct nth<0, Elem, Elems...> {
    typedef Elem type;
};

template<std::size_t Index, typename Elem, typename... Elems>
struct nth<Index, Elem, Elems...> : nth<Index - 1, Elems...> {};

template<std::size_t Index, typename... Elems>
using nth_t = typename nth<Index, Elems...>::type;

} // namespace internal

template<typename... Elems>
class boxes : private internal::boxes_impl<0, Elems...> {
public:
    using internal::boxes_impl<0, Elems...>::boxes_impl;
    boxes(boxes const&) = default;
    boxes(boxes &&) = default;

    boxes & operator=(boxes const&) = default;
    boxes & operator=(boxes&&) = default;

    template<std::size_t Index>
    internal::nth_t<Index, Elems...> const& get() const& { return *internal::select<Index>(*this); }

    template<std::size_t Index>
    internal::nth_t<Index, Elems...> & get() & { return *internal::select<Index>(*this); }

    template<std::size_t Index>
    internal::nth_t<Index, Elems...> && get() && { return *std::move(internal::select<Index>(*this)); }
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BOX_H
