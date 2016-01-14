#ifndef ABB_UND_H
#define ABB_UND_H

namespace abb {

class Und {};


template<typename Value>
struct IsDefined : std::true_type {};

template<>
struct IsDefined<Und> : std::false_type {};

} // namespace abb

#endif // ABB_UND_H