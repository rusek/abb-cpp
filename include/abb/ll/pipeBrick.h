#ifndef ABB_LL_PIPE_BRICK_H
#define ABB_LL_PIPE_BRICK_H

#include <abb/ll/proxyBrick.h>

#include <abb/utils/callReturn.h>
#include <abb/utils/call.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename ContT, typename... ArgsT>
class ContReturn {
private:
    typedef typename utils::CallReturn<ContT, ArgsT...>::element_type ReturnType;
public:
    typedef typename ReturnType::ResultType ResultType;
    typedef typename ReturnType::ReasonType ReasonType;
    typedef Brick<ResultType, ReasonType> BrickType;
};

template<typename ContT, typename ValueT>
struct ContReturn2 {};

template<typename ContT, typename... ArgsT>
struct ContReturn2<ContT, void(ArgsT...)> : ContReturn<ContT, ArgsT...> {};

template<typename ContT, typename ValueT>
using ContBrick = typename ContReturn2<ContT, ValueT>::BrickType;

} // namespace internal

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
class PipeBrick : public internal::ContBrick<SuccessContT, ResultT>, private Successor {
private:
    typedef Brick<ResultT, ReasonT> InBrickType;
    typedef internal::ContBrick<SuccessContT, ResultT> BrickType;
    typedef internal::ContBrick<ErrorContT, ReasonT> ErrorBrickType;
    static_assert(
        std::is_same<BrickType, ErrorBrickType>::value,
        "Continuations should return same brick type"
    );

public:
    PipeBrick(
        std::unique_ptr<InBrickType> inBrick,
        SuccessContT successCont,
        ErrorContT errorCont
    );

    virtual void setSuccessor(Successor & successor);
    virtual bool hasResult() const;
    virtual ValueToTuple<ResultT> & getResult();
    virtual bool hasReason() const;
    virtual ValueToTuple<ReasonT> & getReason();

private:
    virtual void oncomplete();

    std::unique_ptr<InBrickType> inBrick;
    SuccessContT successCont;
    ErrorContT errorCont;
    ProxyBrick<typename BrickType::ResultType, typename BrickType::ReasonType> proxyBrick;
};

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::PipeBrick(
    std::unique_ptr<InBrickType> inBrick,
    SuccessContT successCont,
    ErrorContT errorCont
):
    inBrick(std::move(inBrick)),
    successCont(std::move(successCont)),
    errorCont(std::move(errorCont))
{
    this->inBrick->setSuccessor(*this);
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
void PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::oncomplete() {
    if (this->inBrick->hasResult()) {
        this->proxyBrick.setBrick(utils::call(this->successCont, std::move(this->inBrick->getResult())));
    } else {
        this->proxyBrick.setBrick(utils::call(this->errorCont, std::move(this->inBrick->getReason())));
    }
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
void PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::setSuccessor(Successor & successor) {
    this->proxyBrick.setSuccessor(successor);
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
bool PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::hasResult() const {
    return this->proxyBrick.hasResult();
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
ValueToTuple<ResultT> & PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::getResult() {
    return this->proxyBrick.getResult();
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
bool PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::hasReason() const {
    return this->proxyBrick.hasReason();
}

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
ValueToTuple<ReasonT> & PipeBrick<ResultT, ReasonT, SuccessContT, ErrorContT>::getReason() {
    return this->proxyBrick.getReason();
}


template<typename ResultT, typename SuccessContT>
class PipeBrick<ResultT, Und, SuccessContT, Und> :
    public internal::ContBrick<SuccessContT, ResultT>,
    private Successor
{
private:
    typedef Brick<ResultT, Und> InBrickType;
    typedef internal::ContBrick<SuccessContT, ResultT> BrickType;

public:
    PipeBrick(std::unique_ptr<InBrickType> inBrick, SuccessContT successCont, Und);

    virtual void setSuccessor(Successor & successor);
    virtual bool hasResult() const;
    virtual ValueToTuple<ResultT> & getResult();

private:
    virtual void oncomplete();

    std::unique_ptr<InBrickType> inBrick;
    SuccessContT successCont;
    ProxyBrick<typename BrickType::ResultType, typename BrickType::ReasonType> proxyBrick;
};

template<typename ResultT, typename SuccessContT>
void PipeBrick<ResultT, Und, SuccessContT, Und>::oncomplete() {
    this->proxyBrick.setBrick(utils::call(this->successCont, std::move(this->inBrick->getResult())));
}

template<typename ResultT, typename SuccessContT>
PipeBrick<ResultT, Und, SuccessContT, Und>::PipeBrick(
    std::unique_ptr<InBrickType> inBrick,
    SuccessContT successCont,
    Und
):
    inBrick(std::move(inBrick)),
    successCont(std::move(successCont))
{
    this->inBrick->setSuccessor(*this);
}

template<typename ResultT, typename SuccessContT>
void PipeBrick<ResultT, Und, SuccessContT, Und>::setSuccessor(Successor & successor) {
    this->proxyBrick.setSuccessor(successor);
}

template<typename ResultT, typename SuccessContT>
bool PipeBrick<ResultT, Und, SuccessContT, Und>::hasResult() const {
    return this->proxyBrick.hasResult();
}

template<typename ResultT, typename SuccessContT>
ValueToTuple<ResultT> & PipeBrick<ResultT, Und, SuccessContT, Und>::getResult() {
    return this->proxyBrick.getResult();
}


template<typename ReasonT, typename ErrorContT>
class PipeBrick<Und, ReasonT, Und, ErrorContT> :
    public internal::ContBrick<ErrorContT, ReasonT>,
    private Successor
{
private:
    typedef Brick<Und, ReasonT> InBrickType;
    typedef internal::ContBrick<ErrorContT, ReasonT> BrickType;

public:
    PipeBrick(std::unique_ptr<InBrickType> inBrick, Und successCont, ErrorContT errorCont);

    virtual void setSuccessor(Successor & successor);
    virtual bool hasReason() const;
    virtual ValueToTuple<ReasonT> & getReason();

private:
    virtual void oncomplete();

    std::unique_ptr<InBrickType> inBrick;
    ErrorContT errorCont;
    ProxyBrick<typename BrickType::ResultType, typename BrickType::ReasonType> proxyBrick;
};

template<typename ReasonT, typename ErrorContT>
void PipeBrick<Und, ReasonT, Und, ErrorContT>::oncomplete() {
    this->proxyBrick.setBrick(utils::call(this->errorCont, std::move(this->inBrick->getReason())));
}

template<typename ReasonT, typename ErrorContT>
PipeBrick<Und, ReasonT, Und, ErrorContT>::PipeBrick(
    std::unique_ptr<InBrickType> inBrick,
    Und,
    ErrorContT errorCont
):
    inBrick(std::move(inBrick)),
    errorCont(std::move(errorCont)
) {
    this->inBrick->setSuccessor(*this);
}

template<typename ReasonT, typename ErrorContT>
void PipeBrick<Und, ReasonT, Und, ErrorContT>::setSuccessor(Successor & successor) {
    this->proxyBrick.setSuccessor(successor);
}

template<typename ReasonT, typename ErrorContT>
bool PipeBrick<Und, ReasonT, Und, ErrorContT>::hasReason() const {
    return this->proxyBrick.hasReason();
}

template<typename ReasonT, typename ErrorContT>
ValueToTuple<ReasonT> & PipeBrick<Und, ReasonT, Und, ErrorContT>::getReason() {
    return this->proxyBrick.getReason();
}


} // namespace ll
} // namespace abb

#endif // ABB_LL_PIPE_BRICK_H
