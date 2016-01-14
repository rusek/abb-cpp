#ifndef ABB_LL_SUCCESSOR_H
#define ABB_LL_SUCCESSOR_H

#include <abb/und.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class Successor {
public:
};

template<typename... ArgsT>
class Successor<void(ArgsT...), Und> {
public:
    typedef void ResultType(ArgsT...);
    typedef Und ReasonType;

    virtual ~Successor() {}

    virtual void onsuccess(ArgsT...) = 0;
};

template<typename... ArgsT>
class Successor<Und, void(ArgsT...)> {
public:
    typedef Und ResultType;
    typedef void ReasonTypes(ArgsT...);

    virtual ~Successor() {}

    virtual void onerror(ArgsT...) = 0;
};

template<typename... ResultArgsT, typename... ReasonArgsT>
class Successor<void(ResultArgsT...), void(ReasonArgsT...)> :
        public Successor<void(ResultArgsT...), Und>,
        public Successor<Und, void(ReasonArgsT...)> {
public:
    typedef void ResultType(ResultArgsT...);
    typedef void ReasonType(ReasonArgsT...);
};

} // namespace ll
} // namespace abb

#endif // ABB_SUCCESSOR_H
