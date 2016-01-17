#ifndef ABB_LL_SUCCESSOR_H
#define ABB_LL_SUCCESSOR_H

#include <abb/und.h>

namespace abb {
namespace ll {

class Successor {
public:
    virtual ~Successor() {}
    virtual void oncomplete() = 0;
};

} // namespace ll
} // namespace abb

#endif // ABB_SUCCESSOR_H
