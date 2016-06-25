#ifndef ABB_LL_PIPE_BRICK_H
#define ABB_LL_PIPE_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/brick_cont.h>

#include <abb/value.h>

#include <type_traits>

namespace abb {
namespace ll {

namespace internal {

template<typename Result, typename Reason, typename SuccessCont, typename ErrorCont>
struct pipe_traits {
    typedef brick_cont<Result, Reason, SuccessCont, ErrorCont> brick_cont_type;
    typedef typename brick_cont_type::in_brick_ptr_type in_brick_ptr_type;
    typedef typename brick_cont_type::out_brick_ptr_type out_brick_ptr_type;
    typedef brick<get_result_t<out_brick_ptr_type>, get_reason_t<out_brick_ptr_type>> base_type;
};

} // namespace internal

template<typename Result, typename Reason, typename SuccessCont, typename ErrorCont, typename AbortCont>
class pipe_brick :
    public internal::pipe_traits<Result, Reason, SuccessCont, ErrorCont>::base_type,
    private successor
{
private:
    typedef internal::pipe_traits<Result, Reason, SuccessCont, ErrorCont> traits;
    typedef typename traits::in_brick_ptr_type in_brick_ptr_type;
    typedef typename traits::out_brick_ptr_type out_brick_ptr_type;
    typedef typename traits::brick_cont_type brick_cont_type;

public:
    template<typename SuccessArg, typename ErrorArg>
    pipe_brick(
        in_brick_ptr_type in_brick,
        SuccessArg && success_arg,
        ErrorArg && error_arg,
        AbortCont abort_cont
    ):
        in_brick(std::move(in_brick)),
        success_error_cont(std::forward<SuccessArg>(success_arg), std::forward<ErrorArg>(error_arg)),
        abort_cont(std::move(abort_cont)),
        succ(nullptr)
    {
    }

    void abort() {
        this->in_brick.abort();
    }

    void start(successor & succ) {
        this->succ = &succ;
        this->on_update();
    }

    status get_status() const {
        return this->out_brick ? next_status : pending_status;
    }

    out_brick_ptr_type get_next() {
        return std::move(this->out_brick);
    }

private:
    virtual void on_update();
    virtual island & get_island() const;
    virtual bool is_aborted() const;

    in_brick_ptr_type in_brick;
    brick_cont_type success_error_cont;
    AbortCont abort_cont;
    out_brick_ptr_type out_brick;
    successor * succ;
};

template<typename Result, typename Reason, typename SuccessCont, typename ErrorCont, typename AbortCont>
void pipe_brick<Result, Reason, SuccessCont, ErrorCont, AbortCont>::on_update() {
    if (status in_status = this->in_brick.try_start(*this)) {
        if (in_status & abort_status) {
            this->out_brick = this->abort_cont();
        } else {
            this->out_brick = std::move(this->success_error_cont)(std::move(this->in_brick));
        }
        this->succ->on_update();
    }
}

template<typename Result, typename Reason, typename SuccessCont, typename ErrorCont, typename AbortCont>
island & pipe_brick<Result, Reason, SuccessCont, ErrorCont, AbortCont>::get_island() const {
    return this->succ->get_island();
}

template<typename Result, typename Reason, typename SuccessCont, typename ErrorCont, typename AbortCont>
bool pipe_brick<Result, Reason, SuccessCont, ErrorCont, AbortCont>::is_aborted() const {
    return this->succ->is_aborted();
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PIPE_BRICK_H
