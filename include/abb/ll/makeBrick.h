#ifndef ABB_LL_MAKE_BRICK_H
#define ABB_LL_MAKE_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT, typename FuncT>
class MakeBrick : public Brick<ResultT, ReasonT>, private Successor {
public:
    template<typename... ArgsT>
    MakeBrick(ArgsT &&... args): func(std::forward<ArgsT>(args)...), successor(nullptr) {}

    void start(Successor & successor) {
        this->successor = &successor;
        this->brick = this->func();
        this->onUpdate();
    }

    void abort() {
        this->brick.abort();
    }

    Status getStatus() const {
        return this->brick && (this->brick.getStatus() & (SUCCESS | ERROR | ABORT)) ? NEXT : PENDING;
    }

    BrickPtr<ResultT, ReasonT> getNext() {
        return std::move(this->brick);
    }

private:
    virtual void onUpdate();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;

    FuncT func;
    Successor * successor;
    BrickPtr<ResultT, ReasonT> brick;
};

template<typename ResultT, typename ReasonT, typename FuncT>
void MakeBrick<ResultT, ReasonT, FuncT>::onUpdate() {
    for (;;) {
        Status status = this->brick.getStatus();
        if (status == PENDING) {
            this->brick.start(*this);
            return;
        } else if (status == NEXT) {
            this->brick = this->brick.getNext();
        } else {
            this->successor->onUpdate();
            return;
        }
    }
}

template<typename ResultT, typename ReasonT, typename FuncT>
Island & MakeBrick<ResultT, ReasonT, FuncT>::getIsland() const {
    return this->successor->getIsland();
}

template<typename ResultT, typename ReasonT, typename FuncT>
bool MakeBrick<ResultT, ReasonT, FuncT>::isAborted() const {
    return this->successor->isAborted();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_MAKE_BRICK_H
