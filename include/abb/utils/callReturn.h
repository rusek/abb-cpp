#ifndef ABB_UTILS_CALL_RESULT_H
#define ABB_UTILS_CALL_RESULT_H

#include <type_traits>

namespace abb {
namespace utils {

template<typename FuncT, typename... ArgsT>
using CallReturn = typename std::result_of<FuncT(ArgsT...)>::type;

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_CALL_RESULT_H
