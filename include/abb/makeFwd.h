#ifndef ABB_MAKE_FWD_H
#define ABB_MAKE_FWD_H

#include <functional>

namespace abb {

namespace internal {

template<typename FuncT>
using MakeReturn = typename std::result_of<FuncT()>::type;

} // namespace internal

template<typename FuncT, typename... ArgsT>
internal::MakeReturn<FuncT> make(ArgsT &&... args);

} // namespace abb

#endif // ABB_MAKE_FWD_H
