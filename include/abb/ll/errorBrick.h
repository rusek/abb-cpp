#ifndef ABB_LL_ERROR_BRICK_H
#define ABB_LL_ERROR_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class ErrorBrick : public Brick<ResultT, ReasonT>, private Task {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    template<typename... ArgsT>
    explicit ErrorBrick(ArgsT &&... args):
        reason(std::forward<ArgsT>(args)...),
        status(ERROR),
        successor(nullptr) {}

    void abort() {
        this->status |= ABORT;
    }

    void setSuccessor(Successor & successor) {
        ABB_ASSERT(!this->successor, "Already got successor");
        this->successor = &successor;
        Island::current().enqueue(*this);
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
    virtual void run();

    ValueToTuple<ReasonType> reason;
    Status status;
    Successor * successor;
};

template<typename ResultT, typename ReasonT>
void ErrorBrick<ResultT, ReasonT>::run() {
    this->successor->oncomplete();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_ERROR_BRICK_H
