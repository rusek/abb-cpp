#ifndef ABB_LL_BRICK_CONT_H
#define ABB_LL_BRICK_CONT_H

#include <abb/ll/brick_ptr.h>

#include <abb/value.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename Value, typename Cont>
struct cont_return {};

template<typename... Args, typename Cont>
struct cont_return<void(Args...), Cont> {
    typedef typename std::result_of<Cont &&(Args...)>::type type;
};

template<typename Value, typename Cont>
using cont_return_t = typename cont_return<Value, Cont>::type;

} // namespace internal

template<typename Result, typename Cont>
class success_brick_cont {
public:
    typedef brick_ptr<Result, und_t> in_brick_ptr_type;
    typedef internal::cont_return_t<Result, Cont> out_brick_ptr_type;

    template<typename Arg>
    success_brick_cont(Arg && arg): cont(std::forward<Arg>(arg)) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) && {
        return std::move(*this).call(in_brick.get_result(), get_store_getters_t<Result>());
    }

private:
    template<typename... Getters>
    out_brick_ptr_type call(store<Result> & store, store_getters<Getters...> ) && {
        return std::move(*this).cont(Getters()(std::move(store))...);
    }

    Cont cont;
};

template<typename Result>
class success_brick_cont<Result, pass_t> {
public:
    typedef brick_ptr<Result, und_t> in_brick_ptr_type;
    typedef in_brick_ptr_type out_brick_ptr_type;

    success_brick_cont(pass_t) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) {
        return std::move(in_brick);
    }
};

template<typename Reason, typename Cont>
class error_brick_cont {
public:
    typedef brick_ptr<und_t, Reason> in_brick_ptr_type;
    typedef internal::cont_return_t<Reason, Cont> out_brick_ptr_type;

    template<typename Arg>
    error_brick_cont(Arg && arg): cont(std::forward<Arg>(arg)) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) && {
        return std::move(*this).call(in_brick.get_reason(), get_store_getters_t<Reason>());
    }

private:
    template<typename... Getters>
    out_brick_ptr_type call(store<Reason> & store, store_getters<Getters...> ) && {
        return std::move(this->cont)(Getters()(std::move(store))...);
    }

    Cont cont;
};

template<typename Reason>
class error_brick_cont<Reason, pass_t> {
public:
    typedef brick_ptr<und_t, Reason> in_brick_ptr_type;
    typedef in_brick_ptr_type out_brick_ptr_type;

    error_brick_cont(pass_t) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) {
        return std::move(in_brick);
    }
};

template<typename Result, typename Reason, typename SuccessCont, typename ErrorCont>
class brick_cont {
public:
    typedef success_brick_cont<Result, SuccessCont> success_brick_cont_type;
    typedef error_brick_cont<Reason, ErrorCont> error_brick_cont_type;
    typedef brick_ptr<Result, Reason> in_brick_ptr_type;
    typedef common_brick_ptr_t<
        typename success_brick_cont_type::out_brick_ptr_type,
        typename error_brick_cont_type::out_brick_ptr_type
    > out_brick_ptr_type;

    template<typename SuccessArg, typename ErrorArg>
    brick_cont(SuccessArg && success_arg, ErrorArg && error_arg):
        success_cont(std::forward<SuccessArg>(success_arg)),
        error_cont(std::forward<ErrorArg>(error_arg)) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) && {
        status in_status = in_brick.get_status();
        if (in_status == status::abort) {
            return abort_cast(std::move(in_brick));
        } else if (in_status == status::success) {
            return std::move(this->success_cont)(success_cast(std::move(in_brick)));
        } else {
            return std::move(this->error_cont)(error_cast(std::move(in_brick)));
        }
    }

private:
    success_brick_cont_type success_cont;
    error_brick_cont_type error_cont;
};

template<typename Result, typename SuccessCont, typename ErrorCont>
class brick_cont<Result, und_t, SuccessCont, ErrorCont> {
public:
    typedef success_brick_cont<Result, SuccessCont> success_brick_cont_type;
    typedef brick_ptr<Result, und_t> in_brick_ptr_type;
    typedef typename success_brick_cont_type::out_brick_ptr_type out_brick_ptr_type;

    static_assert(is_special<ErrorCont>::value, "Unexpected error continuation");

    template<typename SuccessArg, typename ErrorArg>
    brick_cont(SuccessArg && success_arg, ErrorArg &&):
        success_cont(std::forward<SuccessArg>(success_arg)) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) && {
        status in_status = in_brick.get_status();
        if (in_status == status::abort) {
            return abort_cast(std::move(in_brick));
        } else {
            return std::move(this->success_cont)(std::move(in_brick));
        }
    }

private:
    success_brick_cont_type success_cont;
};

template<typename Reason, typename SuccessCont, typename ErrorCont>
class brick_cont<und_t, Reason, SuccessCont, ErrorCont> {
public:
    typedef error_brick_cont<Reason, ErrorCont> error_brick_cont_type;
    typedef brick_ptr<und_t, Reason> in_brick_ptr_type;
    typedef typename error_brick_cont_type::out_brick_ptr_type out_brick_ptr_type;

    static_assert(is_special<SuccessCont>::value, "Unexpected success continuation");

    template<typename SuccessArg, typename ErrorArg>
    brick_cont(SuccessArg &&, ErrorArg && error_arg):
        error_cont(std::forward<ErrorArg>(error_arg)) {}

    out_brick_ptr_type operator()(in_brick_ptr_type in_brick) && {
        status in_status = in_brick.get_status();
        if (in_status == status::abort) {
            return abort_cast(std::move(in_brick));
        } else {
            return std::move(this->error_cont)(std::move(in_brick));
        }
    }

private:
    error_brick_cont_type error_cont;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_CONT_H
