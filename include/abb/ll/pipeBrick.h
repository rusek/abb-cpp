#ifndef ABB_LL_PIPE_BRICK_H
#define ABB_LL_PIPE_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/brickCont.h>

#include <abb/value.h>

#include <abb/utils/call.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
struct PipeTraits {
    typedef BrickCont<ResultT, ReasonT, SuccessContT, ErrorContT> BrickContType;
    typedef typename BrickContType::InBrickPtrType InBrickPtrType;
    typedef typename BrickContType::OutBrickPtrType OutBrickPtrType;
    typedef Brick<GetResult<OutBrickPtrType>, GetReason<OutBrickPtrType>> BrickBaseType;
};

} // namespace internal

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT, typename AbortContT>
class PipeBrick :
    public internal::PipeTraits<ResultT, ReasonT, SuccessContT, ErrorContT>::BrickBaseType,
    private Successor
{
private:
    typedef internal::PipeTraits<ResultT, ReasonT, SuccessContT, ErrorContT> Traits;
    typedef typename Traits::InBrickPtrType InBrickPtrType;
    typedef typename Traits::OutBrickPtrType OutBrickPtrType;
    typedef typename Traits::BrickContType BrickContType;

public:
    template<typename SuccessArg, typename ErrorArg>
    PipeBrick(
        InBrickPtrType inBrick,
        SuccessArg && successArg,
        ErrorArg && errorArg,
        AbortContT abortCont
    ):
        inBrick(std::move(inBrick)),
        brickCont(std::forward<SuccessArg>(successArg), std::forward<ErrorArg>(errorArg)),
        abortCont(std::move(abortCont)),
        successor(nullptr)
    {
    }

    void abort() {
        if (this->outBrick) {
            this->outBrick.abort();
        } else {
            this->inBrick.abort();
        }
    }

    void start(Successor & successor) {
        this->successor = &successor;
        this->inBrick.start(*this);
    }

    Status getStatus() const {
        return this->outBrick ? this->outBrick.getStatus() : PENDING;
    }

    ValueToTuple<GetResult<OutBrickPtrType>> & getResult() {
        return this->outBrick.getResult();
    }

    ValueToTuple<GetReason<OutBrickPtrType>> & getReason() {
        return this->outBrick.getReason();
    }

private:
    virtual void oncomplete();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;

    InBrickPtrType inBrick;
    BrickContType brickCont;
    AbortContT abortCont;
    OutBrickPtrType outBrick;
    Successor * successor;
};

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT, typename AbortContT>
void PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT, AbortContT>::oncomplete() {
    if (this->inBrick.getStatus() & ABORT) {
        this->outBrick = this->abortCont();
    } else {
        this->outBrick = this->brickCont(std::move(this->inBrick));
    }

    if (this->outBrick.getStatus() == PENDING) {
        this->outBrick.start(*this->successor);
    } else {
        // this->brickCont returned this->inBrick as this->outBrick
        this->successor->oncomplete();
    }
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT, typename AbortContT>
Island & PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT, AbortContT>::getIsland() const {
    return this->successor->getIsland();
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT, typename AbortContT>
bool PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT, AbortContT>::isAborted() const {
    return this->successor->isAborted();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PIPE_BRICK_H
