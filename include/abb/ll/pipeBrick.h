#ifndef ABB_LL_PIPE_BRICK_H
#define ABB_LL_PIPE_BRICK_H

#include <abb/ll/proxyBrick.h>

#include <abb/utils/callResult.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename ContT, typename... ArgsT>
class ContReturn {
private:
    typedef typename utils::CallResult<ContT, ArgsT...>::Type::element_type ReturnType;
public:
    typedef typename ReturnType::ResultType ResultType;
    typedef typename ReturnType::ReasonType ReasonType;
    typedef Brick<ResultType, ReasonType> BrickType;
};

} // namespace internal

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
class PipeBrick {};

template<typename... ResultArgsT, typename SuccessContT>
class PipeBrick<void(ResultArgsT...), Und, SuccessContT, Und> :
    public internal::ContReturn<SuccessContT, ResultArgsT...>::BrickType,
    private Successor<void(ResultArgsT...), Und>
{
private:
    typedef Brick<void(ResultArgsT...), Und> InBrickType;
    typedef typename internal::ContReturn<SuccessContT, ResultArgsT...>::BrickType BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

public:
    PipeBrick(std::unique_ptr<InBrickType> inBrick, SuccessContT successCont, Und);

    virtual void setSuccessor(SuccessorType & successor);

private:
    virtual void onsuccess(ResultArgsT...);

    std::unique_ptr<InBrickType> inBrick;
    SuccessContT successCont;
    ProxyBrick<typename BrickType::ResultType, typename BrickType::ReasonType> proxyBrick;
};

template<typename... ResultArgsT, typename SuccessContT>
void PipeBrick<void(ResultArgsT...), Und, SuccessContT, Und>::onsuccess(ResultArgsT... args) {
    this->proxyBrick.setBrick(this->successCont(std::forward<ResultArgsT>(args)...));
}

template<typename... ResultArgsT, typename SuccessContT>
PipeBrick<void(ResultArgsT...), Und, SuccessContT, Und>::PipeBrick(
    std::unique_ptr<InBrickType> inBrick,
    SuccessContT successCont,
    Und
):
    inBrick(std::move(inBrick)),
    successCont(std::move(successCont))
{
    this->inBrick->setSuccessor(*this);
}

template<typename... ResultArgsT, typename SuccessContT>
void PipeBrick<void(ResultArgsT...), Und, SuccessContT, Und>::setSuccessor(SuccessorType & successor) {
    this->proxyBrick.setSuccessor(successor);
}


template<typename... ReasonArgsT, typename ErrorContT>
class PipeBrick<Und, void(ReasonArgsT...), Und, ErrorContT> :
    public internal::ContReturn<ErrorContT, ReasonArgsT...>::BrickType,
    private Successor<Und, void(ReasonArgsT...)>
{
private:
    typedef Brick<Und, void(ReasonArgsT...)> InBrickType;
    typedef typename internal::ContReturn<ErrorContT, ReasonArgsT...>::BrickType BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

public:
    PipeBrick(std::unique_ptr<InBrickType> inBrick, Und successCont, ErrorContT errorCont);

    virtual void setSuccessor(SuccessorType & successor);

private:
    virtual void onerror(ReasonArgsT...);

    std::unique_ptr<InBrickType> inBrick;
    ErrorContT errorCont;
    ProxyBrick<typename BrickType::ResultType, typename BrickType::ReasonType> proxyBrick;
};

template<typename... ReasonArgsT, typename ErrorContT>
void PipeBrick<Und, void(ReasonArgsT...), Und, ErrorContT>::onerror(ReasonArgsT... args) {
    this->proxyBrick.setBrick(this->errorCont(std::forward<ReasonArgsT>(args)...));
}

template<typename... ReasonArgsT, typename ErrorContT>
PipeBrick<Und, void(ReasonArgsT...), Und, ErrorContT>::PipeBrick(
    std::unique_ptr<InBrickType> inBrick,
    Und,
    ErrorContT errorCont
):
    inBrick(std::move(inBrick)),
    errorCont(std::move(errorCont)
) {
    this->inBrick->setSuccessor(*this);
}

template<typename... ReasonArgsT, typename ErrorContT>
void PipeBrick<Und, void(ReasonArgsT...), Und, ErrorContT>::setSuccessor(SuccessorType & successor) {
    this->proxyBrick.setSuccessor(successor);
}


template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
class PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT> :
    public internal::ContReturn<SuccessContT, ResultArgsT...>::BrickType,
    private Successor<void(ResultArgsT...), void(ReasonArgsT...)>
{
private:
    typedef Brick<void(ResultArgsT...), void(ReasonArgsT...)> InBrickType;
    typedef internal::ContReturn<SuccessContT, ResultArgsT...> SuccessReturn;
    typedef internal::ContReturn<ErrorContT, ReasonArgsT...> ErrorReturn;
    static_assert(
        std::is_same<typename SuccessReturn::BrickType, typename ErrorReturn::BrickType>::value,
        "Continuations should return same brick type"
    );
    typedef typename SuccessReturn::BrickType BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

public:
    PipeBrick(
        std::unique_ptr<InBrickType> inBrick,
        SuccessContT successCont,
        ErrorContT errorCont
    );

    virtual void setSuccessor(SuccessorType & successor);

private:
    virtual void onsuccess(ResultArgsT...);
    virtual void onerror(ReasonArgsT...);

    std::unique_ptr<InBrickType> inBrick;
    SuccessContT successCont;
    ErrorContT errorCont;
    ProxyBrick<typename BrickType::ResultType, typename BrickType::ReasonType> proxyBrick;
};

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
void PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT>::onsuccess(ResultArgsT... args) {
    this->proxyBrick.setBrick(this->successCont(std::forward<ResultArgsT...>(args)...));
}

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
void PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT>::onerror(ReasonArgsT... args) {
    this->proxyBrick.setBrick(this->errorCont(std::forward<ReasonArgsT>(args)...));
}

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT>::PipeBrick(
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

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
void PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT>::setSuccessor(SuccessorType & successor) {
    this->proxyBrick.setSuccessor(successor);
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PIPE_BRICK_H
