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

typedef int State;

enum StateFlags {
    NEW_STATE = 0,
    SUCCESS_STATE = 1,
    ERROR_STATE = 2,
    DONE_STATE = 4 // FIXME remove
};

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
class ValueBrick : public Brick<ResultT, ReasonT> {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    ValueBrick();

    ~ValueBrick();

    void setSuccessor(Successor & successor);

    template<typename... ArgsT>
    void setResult(ArgsT &&... args);

    template<typename... ArgsT>
    void setReason(ArgsT &&... args);

    bool hasResult() const {
        return this->state & internal::SUCCESS_STATE;
    }

    ValueToTuple<ResultT> & getResult() {
        ABB_ASSERT(this->state & internal::SUCCESS_STATE, "Result is not set");
        return *this->value.result;
    }

    bool hasReason() const {
        return this->state & internal::ERROR_STATE;
    }

    ValueToTuple<ReasonT> & getReason() {
        ABB_ASSERT(this->state & internal::ERROR_STATE, "Reason is not set");
        return *this->value.reason;
    }

protected:
    void complete();

    internal::ValueToBank<ResultType, ReasonType> value;
    internal::State state;
    Successor * successor;
};

template<typename ResultT, typename ReasonT>
ValueBrick<ResultT, ReasonT>::ValueBrick(): state(internal::NEW_STATE), successor(nullptr) {}

template<typename ResultT, typename ReasonT>
ValueBrick<ResultT, ReasonT>::~ValueBrick() {
    ABB_ASSERT(this->state & internal::DONE_STATE, "Not done yet");
    this->value.destroy(this->state & internal::SUCCESS_STATE);
}

template<typename ResultT, typename ReasonT>
void ValueBrick<ResultT, ReasonT>::setSuccessor(Successor & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->state != internal::NEW_STATE) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename ResultT, typename ReasonT>
void ValueBrick<ResultT, ReasonT>::complete() {
    this->state |= internal::DONE_STATE;
    this->successor->oncomplete();
}

template<typename ResultT, typename ReasonT>
template<typename... ArgsT>
void ValueBrick<ResultT, ReasonT>::setResult(ArgsT &&... args) {
    ABB_ASSERT(this->state == internal::NEW_STATE, "Already got value");
    this->value.result.init(std::forward<ArgsT>(args)...);
    this->state = internal::SUCCESS_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename ResultT, typename ReasonT>
template<typename... ArgsT>
void ValueBrick<ResultT, ReasonT>::setReason(ArgsT &&... args) {
    ABB_ASSERT(this->state == internal::NEW_STATE, "Already got value");
    this->value.reason.init(std::forward<ArgsT>(args)...);
    this->state = internal::ERROR_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_VALUE_BRICK_H
