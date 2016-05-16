#ifndef ABB_SPECIAL_H
#define ABB_SPECIAL_H

#include <type_traits>

namespace abb {

class Und {};

class Pass {};

const Pass pass;

template<typename Value>
struct IsDefined : std::true_type {};

template<>
struct IsDefined<Und> : std::false_type {};

template<typename Value>
struct IsPass : std::false_type {};

template<>
struct IsPass<Pass> : std::true_type {};

} // namespace abb

#endif // ABB_SPECIAL_H
