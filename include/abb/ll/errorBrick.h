#ifndef ABB_LL_ERROR_BRICK_H
#define ABB_LL_ERROR_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class ErrorBrick : public Brick<ResultT, ReasonT> {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    template<typename... ArgsT>
    explicit ErrorBrick(ArgsT &&... args):
        reason(std::forward<ArgsT>(args)...),
        status(ERROR) {}

    void abort() {
        this->status |= ABORT;
    }

    void run(Successor & successor) {
        successor.oncomplete();
    }

    Status getStatus() const {
        return this->status;
    }

    ValueToTuple<ResultT> & getResult() {
        ABB_FIASCO("getReason called on ErrorBrick");
    }

    ValueToTuple<ReasonT> & getReason() {
        return this->reason;
    }

private:
    ValueToTuple<ReasonType> reason;
    Status status;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_ERROR_BRICK_H