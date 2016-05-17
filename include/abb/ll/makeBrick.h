#ifndef ABB_LL_MAKE_BRICK_H
#define ABB_LL_MAKE_BRICK_H

#include <abb/ll/brick.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT, typename FuncT>
class MakeBrick : public Brick<ResultT, ReasonT> {
public:
    template<typename... ArgsT>
    MakeBrick(ArgsT &&... args): func(std::forward<ArgsT>(args)...) {}

    void start(Successor & successor) {
        this->brick = this->func();
        this->brick.start(successor);
    }

    void abort() {
        this->brick.abort();
    }

    Status getStatus() const {
        return this->brick.getStatus();
    }

    ValueToTuple<ResultT> & getResult() {
        return this->brick.getResult();
    }

    ValueToTuple<ReasonT> & getReason() {
        return this->brick.getReason();
    }


private:
    FuncT func;
    BrickPtr<ResultT, ReasonT> brick;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_MAKE_BRICK_H
