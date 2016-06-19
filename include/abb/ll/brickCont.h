#ifndef ABB_LL_BRICK_CONT_H
#define ABB_LL_BRICK_CONT_H

#include <abb/ll/brickPtr.h>

#include <abb/value.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename ValueT, typename ContT>
struct ContReturnImpl {};

template<typename... ArgsT, typename ContT>
struct ContReturnImpl<void(ArgsT...), ContT> {
    typedef typename std::result_of<ContT &&(ArgsT...)>::type Type;
};

template<typename ValueT, typename ContT>
using ContReturn = typename ContReturnImpl<ValueT, ContT>::Type;

} // namespace internal

template<typename ResultT, typename ContT>
class SuccessBrickCont {
public:
    typedef BrickPtr<ResultT, Und> InBrickPtrType;
    typedef internal::ContReturn<ResultT, ContT> OutBrickPtrType;

    template<typename ArgT>
    SuccessBrickCont(ArgT && arg): cont(std::forward<ArgT>(arg)) {}

    OutBrickPtrType operator()(InBrickPtrType inBrick) && {
        return std::move(*this).call(inBrick.getResult(), GetStoreGetters<ResultT>());
    }

private:
    template<typename... GettersT>
    OutBrickPtrType call(Store<ResultT> & store, StoreGetters<GettersT...> ) && {
        return std::move(*this).cont(GettersT()(std::move(store))...);
    }

    ContT cont;
};

template<typename ResultT>
class SuccessBrickCont<ResultT, Pass> {
public:
    typedef BrickPtr<ResultT, Und> InBrickPtrType;
    typedef InBrickPtrType OutBrickPtrType;

    SuccessBrickCont(Pass) {}

    OutBrickPtrType operator()(InBrickPtrType inBrick) {
        return std::move(inBrick);
    }
};

template<typename ReasonT, typename ContT>
class ErrorBrickCont {
public:
    typedef BrickPtr<Und, ReasonT> InBrickPtrType;
    typedef internal::ContReturn<ReasonT, ContT> OutBrickPtrType;

    template<typename ArgT>
    ErrorBrickCont(ArgT && arg): cont(std::forward<ArgT>(arg)) {}

    OutBrickPtrType operator()(InBrickPtrType inBrick) && {
        return std::move(*this).call(inBrick.getReason(), GetStoreGetters<ReasonT>());
    }

private:
    template<typename... GettersT>
    OutBrickPtrType call(Store<ReasonT> & store, StoreGetters<GettersT...> ) && {
        return std::move(this->cont)(GettersT()(std::move(store))...);
    }

    ContT cont;
};

template<typename ReasonT>
class ErrorBrickCont<ReasonT, Pass> {
public:
    typedef BrickPtr<Und, ReasonT> InBrickPtrType;
    typedef InBrickPtrType OutBrickPtrType;

    ErrorBrickCont(Pass) {}

    OutBrickPtrType operator()(InBrickPtrType inBrick) {
        return std::move(inBrick);
    }
};

template<typename ResultT, typename ReasonT, typename SuccessContT, typename ErrorContT>
class BrickCont {
public:
    typedef SuccessBrickCont<ResultT, SuccessContT> SuccessBrickContType;
    typedef ErrorBrickCont<ReasonT, ErrorContT> ErrorBrickContType;
    typedef BrickPtr<ResultT, ReasonT> InBrickPtrType;
    typedef CommonBrickPtr<
        typename SuccessBrickContType::OutBrickPtrType,
        typename ErrorBrickContType::OutBrickPtrType
    > OutBrickPtrType;

    template<typename SuccessArgT, typename ErrorArgT>
    BrickCont(SuccessArgT && successArg, ErrorArgT && errorArg):
        successBrickCont(std::forward<SuccessArgT>(successArg)),
        errorBrickCont(std::forward<ErrorArgT>(errorArg)) {}

    OutBrickPtrType operator()(InBrickPtrType inBrick) && {
        if (inBrick.getStatus() & SUCCESS) {
            return std::move(this->successBrickCont)(successCast(std::move(inBrick)));
        } else {
            return std::move(this->errorBrickCont)(errorCast(std::move(inBrick)));
        }
    }

private:
    SuccessBrickContType successBrickCont;
    ErrorBrickContType errorBrickCont;
};

template<typename ResultT, typename SuccessContT, typename ErrorContT>
class BrickCont<ResultT, Und, SuccessContT, ErrorContT> : public SuccessBrickCont<ResultT, SuccessContT> {
public:
    static_assert(IsSpecial<ErrorContT>::value, "Unexpected error continuation");

    template<typename SuccessArgT, typename ErrorArgT>
    BrickCont(SuccessArgT && successArg, ErrorArgT &&):
        SuccessBrickCont<ResultT, SuccessContT>(std::forward<SuccessArgT>(successArg)) {}
};

template<typename ReasonT, typename SuccessContT, typename ErrorContT>
class BrickCont<Und, ReasonT, SuccessContT, ErrorContT> : public ErrorBrickCont<ReasonT, ErrorContT> {
public:
    static_assert(IsSpecial<SuccessContT>::value, "Unexpected error continuation");

    template<typename SuccessArgT, typename ErrorArgT>
    BrickCont(SuccessArgT &&, ErrorArgT && errorArg):
        ErrorBrickCont<ReasonT, ErrorContT>(std::forward<ErrorArgT>(errorArg)) {}
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_CONT_H
