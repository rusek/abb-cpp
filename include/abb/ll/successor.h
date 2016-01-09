#ifndef ABB_LL_SUCCESSOR_H
#define ABB_LL_SUCCESSOR_H

namespace abb {
namespace ll {

template<typename DoneCont>
class Successor {
public:
};

template<typename... Args>
class Successor<void(Args...)> {
public:
    virtual ~Successor() {}

    virtual void done(Args...) = 0;
};

} // namespace ll
} // namespace abb

#endif // ABB_SUCCESSOR_H
