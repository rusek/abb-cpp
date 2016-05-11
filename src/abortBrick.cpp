#include <abb/ll/abortBrick.h>

namespace abb {
namespace ll {

void AbortBrick::run() {
    this->successor->oncomplete();
}

} // namespace ll
} // namespace abb
