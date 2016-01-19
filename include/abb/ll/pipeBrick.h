#ifndef ABB_LL_PIPE_BRICK_H
#define ABB_LL_PIPE_BRICK_H

#include <abb/ll/proxyBrick.h>

#include <abb/utils/callReturn.h>
#include <abb/utils/call.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
class ContPair :
    public ContPair<ResultT, Und, SuccessContT, Und>,
    public ContPair<Und, ReasonT, Und, ErrorContT>
{
private:
    typedef ContPair<ResultT, Und, SuccessContT, Und> SuccessBase;
    typedef ContPair<Und, ReasonT, Und, ErrorContT> ErrorBase;

public:
    typedef typename SuccessBase::BrickPtrType BrickPtrType;
    typedef typename SuccessBase::BrickType BrickType;
    static_assert(
        std::is_same<BrickPtrType, typename ErrorBase::BrickPtrType>::value,
        "Continuations should return same brick type"
    );

    ContPair(SuccessContT successCont, ErrorContT errorCont):
        SuccessBase(successCont, Und()),
        ErrorBase(Und(), errorCont) {}

    BrickPtrType call(BrickPtr<ResultT, ReasonT> & inBrick) {
        return inBrick.hasResult() ?
            this->SuccessBase::call(inBrick) :
            this->ErrorBase::call(inBrick);
    }
};

template<typename... ResultArgsT, typename SuccessContT>
class ContPair<void(ResultArgsT...), Und, SuccessContT, Und> {
public:
    typedef utils::CallReturn<SuccessContT, ResultArgsT...> BrickPtrType;
    typedef Brick<typename BrickPtrType::ResultType, typename BrickPtrType::ReasonType> BrickType;

    ContPair(SuccessContT successCont, Und): successCont(std::move(successCont)) {}

    template<typename ReasonT>
    BrickPtrType call(BrickPtr<void(ResultArgsT...), ReasonT> & inBrick) {
        return utils::call(this->successCont, std::move(inBrick.getResult()));
    }

private:
    SuccessContT successCont;
};

template<typename... ReasonArgsT, typename ErrorContT>
class ContPair<Und, void(ReasonArgsT...), Und, ErrorContT> {
public:
    typedef utils::CallReturn<ErrorContT, ReasonArgsT...> BrickPtrType;
    typedef Brick<typename BrickPtrType::ResultType, typename BrickPtrType::ReasonType> BrickType;

    ContPair(Und, ErrorContT errorCont): errorCont(std::move(errorCont)) {}

    template<typename ResultT>
    BrickPtrType call(BrickPtr<ResultT, void(ReasonArgsT...)> & inBrick) {
        return utils::call(this->errorCont, std::move(inBrick.getReason()));
    }

private:
    ErrorContT errorCont;
};

} // namespace internal

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
class PipeBrick :
    public internal::ContPair<ResultT, ReasonT, SuccessContT, ErrorContT>::BrickType,
    private Successor
{
private:
    typedef BrickPtr<ResultT, ReasonT> InBrickPtrType;
    typedef internal::ContPair<ResultT, ReasonT, SuccessContT, ErrorContT> ContPairType;
    typedef typename ContPairType::BrickPtrType BrickPtrType;

public:
    PipeBrick(
        InBrickPtrType inBrick,
        SuccessContT successCont,
        ErrorContT errorCont
    );

    void setSuccessor(Successor & successor) {
        this->proxyBrick.setSuccessor(successor);
    }

    bool hasResult() const {
        return this->proxyBrick.hasResult();
    }

    ValueToTuple<typename BrickPtrType::ResultType> & getResult() {
        return this->proxyBrick.getResult();
    }

    bool hasReason() const {
        return this->proxyBrick.hasReason();
    }

    ValueToTuple<typename BrickPtrType::ReasonType> & getReason() {
        return this->proxyBrick.getReason();
    }

private:
    virtual void oncomplete();

    InBrickPtrType inBrick;
    ContPairType contPair;
    ProxyBrick<typename BrickPtrType::ResultType, typename BrickPtrType::ReasonType> proxyBrick;
};

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::PipeBrick(
    InBrickPtrType inBrick,
    SuccessContT successCont,
    ErrorContT errorCont
):
    inBrick(std::move(inBrick)),
    contPair(std::move(successCont), std::move(errorCont))
{
    this->inBrick.setSuccessor(*this);
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
void PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::oncomplete() {
    this->proxyBrick.setBrick(this->contPair.call(this->inBrick));
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PIPE_BRICK_H
