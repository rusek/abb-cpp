#ifndef ABB_LL_SUCCESS_BRICK_H
#define ABB_LL_SUCCESS_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class SuccessBrick : public Brick<ResultT, ReasonT> {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    template<typename... ArgsT>
    explicit SuccessBrick(ArgsT &&... args):
        result(std::forward<ArgsT>(args)...) {}

    void abort() {}

    void start(Successor &) {
        ABB_FIASCO("start called on SuccessBrick");
    }

    Status getStatus() const {
        return SUCCESS;
    }

    Store<ResultT> & getResult() {
        return this->result;
    }

    Store<ReasonT> & getReason() {
        ABB_FIASCO("getReason called on SuccessBrick");
    }

private:
    Store<ResultType> result;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H
