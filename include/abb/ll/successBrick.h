#ifndef ABB_LL_SUCCESS_BRICK_H
#define ABB_LL_SUCCESS_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class SuccessBrick : public Brick<ResultT, ReasonT>, private Task {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    template<typename... ArgsT>
    explicit SuccessBrick(ArgsT &&... args):
        result(std::forward<ArgsT>(args)...),
        status(SUCCESS),
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
        return this->result;
    }

    ValueToTuple<ReasonT> & getReason() {
        ABB_FIASCO("getReason called on SuccessBrick");
    }

private:
    virtual void run();

    ValueToTuple<ResultType> result;
    Status status;
    Successor * successor;
};

template<typename ResultT, typename ReasonT>
void SuccessBrick<ResultT, ReasonT>::run() {
    this->successor->oncomplete();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H
