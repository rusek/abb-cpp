#ifndef ABB_LL_SUCCESSOR_H
#define ABB_LL_SUCCESSOR_H

#include <abb/special.h>
#include <abb/island.h>

namespace abb {
namespace ll {

class Successor {
public:
    virtual ~Successor() {}
    virtual void oncomplete() = 0;
    virtual Island & getIsland() const = 0;
};

} // namespace ll
} // namespace abb

#endif // ABB_SUCCESSOR_H
