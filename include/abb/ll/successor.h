#ifndef ABB_LL_SUCCESSOR_H
#define ABB_LL_SUCCESSOR_H

#include <abb/und.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class Successor {
public:
};

template<typename... Args>
class Successor<void(Args...), Und> {
public:
    virtual ~Successor() {}

    virtual void onsuccess(Args...) = 0;
};

template<typename... Args>
class Successor<Und, void(Args...)> {
public:
    virtual ~Successor() {}

    virtual void onerror(Args...) = 0;
};

template<typename... ResultArgs, typename... ReasonArgs>
class Successor<void(ResultArgs...), void(ReasonArgs...)> :
        public Successor<void(ResultArgs...), Und>,
        public Successor<Und, void(ReasonArgs...)> {
};

} // namespace ll
} // namespace abb

#endif // ABB_SUCCESSOR_H
