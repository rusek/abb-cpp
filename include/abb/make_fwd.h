#ifndef ABB_MAKE_FWD_H
#define ABB_MAKE_FWD_H

#include <functional>

namespace abb {

namespace internal {

template<typename Func>
using make_return_t = typename std::result_of<Func &&()>::type;

} // namespace internal

template<typename Func, typename... Args>
internal::make_return_t<Func> make(Args &&... args);

} // namespace abb

#endif // ABB_MAKE_FWD_H
