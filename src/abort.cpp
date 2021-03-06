#include <abb/abort.h>
#include <abb/ll/abort_point_brick.h>
#include <abb/ll/bridge.h>

namespace abb {
namespace ll {

void abort_point_brick::run() {
    this->cur_status = this->aborted ? status::abort : status::success;
    this->succ->on_update();
}

} // namespace ll


block<void, und_t> abort_point() {
    return ll::pack_brick<ll::abort_point_brick>();
}

} // namespace abb
