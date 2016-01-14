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

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
class PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT> :
        public internal::ContReturn<SuccessContT, ResultArgsT...>::BrickType,
        private Successor<void(ResultArgsT...), void(ReasonArgsT...)> {
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
    this->proxyBrick.setBrick(this->successCont(args...));
}

template<typename... ResultArgsT, typename... ReasonArgsT, typename SuccessContT, typename ErrorContT>
void PipeBrick<void(ResultArgsT...), void(ReasonArgsT...), SuccessContT, ErrorContT>::onerror(ReasonArgsT... args) {
    this->proxyBrick.setBrick(this->errorCont(args...));
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
