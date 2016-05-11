#ifndef ABB_LL_VALUE_BRICK_H
#define ABB_LL_VALUE_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>
#include <abb/utils/bank.h>

#include <tuple>

namespace abb {
namespace ll {

namespace internal {

template<typename ResultT, typename ReasonT>
struct ValueBankImpl {
    union Type {
        void destroy(bool success) {
            if (success) {
                this->result.destroy();
            } else {
                this->reason.destroy();
            }
        }

        utils::Bank<ValueToTuple<ResultT>> result;
        utils::Bank<ValueToTuple<ReasonT>> reason;
    };
};

template<typename ResultT>
struct ValueBankImpl<ResultT, Und> {
    struct Type {
        void destroy(bool) {
            this->result.destroy();
        }

        utils::Bank<ValueToTuple<ResultT>> result;
    };
};

template<typename ReasonT>
struct ValueBankImpl<Und, ReasonT> {
    struct Type {
        void destroy(bool) {
            this->reason.destroy();
        }

        utils::Bank<ValueToTuple<ReasonT>> reason;
    };
};

template<typename ResultT, typename ReasonT>
using ValueToBank = typename ValueBankImpl<ResultT, ReasonT>::Type;

} // namespace internal

template<typename ResultT, typename ReasonT>
class ValueBrick : public Brick<ResultT, ReasonT>, private Task {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    ValueBrick();

    ~ValueBrick();

    void abort() {
        this->status |= ABORT;
    }

    void setSuccessor(Successor & successor);

    Status getStatus() const {
        return this->status;
    }

    template<typename... ArgsT>
    void setResult(ArgsT &&... args);

    template<typename... ArgsT>
    void setReason(ArgsT &&... args);

    ValueToTuple<ResultT> & getResult() {
        ABB_ASSERT(this->status & SUCCESS, "Result is not set");
        return *this->value.result;
    }

    ValueToTuple<ReasonT> & getReason() {
        ABB_ASSERT(this->status & ERROR, "Reason is not set");
        return *this->value.reason;
    }

private:
    virtual void run();

    internal::ValueToBank<ResultType, ReasonType> value;
    Status status;
    Successor * successor;
};

template<typename ResultT, typename ReasonT>
ValueBrick<ResultT, ReasonT>::ValueBrick(): status(PENDING), successor(nullptr) {}

template<typename ResultT, typename ReasonT>
ValueBrick<ResultT, ReasonT>::~ValueBrick() {
    if (this->status & (SUCCESS | ERROR)) {
        this->value.destroy(this->status & SUCCESS);
    }
}

template<typename ResultT, typename ReasonT>
void ValueBrick<ResultT, ReasonT>::setSuccessor(Successor & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->status != PENDING) {
        Island::current().enqueue(*this);
    }
}

template<typename ResultT, typename ReasonT>
void ValueBrick<ResultT, ReasonT>::run() {
    this->successor->oncomplete();
}

template<typename ResultT, typename ReasonT>
template<typename... ArgsT>
void ValueBrick<ResultT, ReasonT>::setResult(ArgsT &&... args) {
    ABB_ASSERT(this->status == PENDING, "Already got value");
    this->value.result.init(std::forward<ArgsT>(args)...);
    this->status = SUCCESS;
    if (this->successor) {
        Island::current().enqueue(*this);
    }
}

template<typename ResultT, typename ReasonT>
template<typename... ArgsT>
void ValueBrick<ResultT, ReasonT>::setReason(ArgsT &&... args) {
    ABB_ASSERT(this->status == PENDING, "Already got value");
    this->value.reason.init(std::forward<ArgsT>(args)...);
    this->status = ERROR;
    if (this->successor) {
        Island::current().enqueue(*this);
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_VALUE_BRICK_H
