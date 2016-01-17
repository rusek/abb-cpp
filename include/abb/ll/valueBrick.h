#ifndef ABB_LL_VALUE_BRICK_H
#define ABB_LL_VALUE_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>
#include <abb/utils/vault.h>

#include <memory>
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
struct ValueVaultImpl {
    union Type {
        void destroy(bool success) {
            if (success) {
                this->result.destroy();
            } else {
                this->reason.destroy();
            }
        }

        utils::Vault<ValueToTuple<ResultT>> result;
        utils::Vault<ValueToTuple<ReasonT>> reason;
    };
};

template<typename ResultT>
struct ValueVaultImpl<ResultT, Und> {
    struct Type {
        void destroy(bool) {
            this->result.destroy();
        }

        utils::Vault<ValueToTuple<ResultT>> result;
    };
};

template<typename ReasonT>
struct ValueVaultImpl<Und, ReasonT> {
    struct Type {
        void destroy(bool) {
            this->reason.destroy();
        }

        utils::Vault<ValueToTuple<ReasonT>> reason;
    };
};

template<typename ResultT, typename ReasonT>
using ValueToVault = typename ValueVaultImpl<ResultT, ReasonT>::Type;


template<typename ResultT, typename ReasonT>
class ValueBrickBase : public Brick<ResultT, ReasonT> {
public:
    typedef Brick<ResultT, ReasonT> BrickType;
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    ValueBrickBase();

    virtual ~ValueBrickBase();

    virtual void setSuccessor(Successor & successor);

protected:
    void complete();

    ValueToVault<ResultType, ReasonType> value;
    State state;
    Successor * successor;
};

template<typename ResultT, typename ReasonT>
ValueBrickBase<ResultT, ReasonT>::ValueBrickBase(): state(NEW_STATE), successor(nullptr) {}

template<typename ResultT, typename ReasonT>
ValueBrickBase<ResultT, ReasonT>::~ValueBrickBase() {
    ABB_ASSERT(this->state & DONE_STATE, "Not done yet");
    this->value.destroy(this->state & SUCCESS_STATE);
}

template<typename ResultT, typename ReasonT>
void ValueBrickBase<ResultT, ReasonT>::setSuccessor(Successor & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->state != NEW_STATE) {
        Island::current().enqueue(std::bind(&ValueBrickBase::complete, this));
    }
}

template<typename ResultT, typename ReasonT>
void ValueBrickBase<ResultT, ReasonT>::complete() {
    this->state |= DONE_STATE;
    this->successor->oncomplete();
}


template<typename ResultT, typename ReasonT>
class ValueBrickSuccess {};

template<typename ReasonT>
class ValueBrickSuccess<Und, ReasonT> : public ValueBrickBase<Und, ReasonT> {};

template<typename... ResultArgsT, typename ReasonT>
class ValueBrickSuccess<void(ResultArgsT...), ReasonT> : public ValueBrickBase<void(ResultArgsT...), ReasonT> {
public:
    void setResult(ResultArgsT... args);

    virtual bool hasResult() const;
    virtual std::tuple<ResultArgsT...> & getResult();
};

template<typename... ResultArgsT, typename ReasonT>
void ValueBrickSuccess<void(ResultArgsT...), ReasonT>::setResult(ResultArgsT... args) {
    ABB_ASSERT(this->state == NEW_STATE, "Already got value");
    this->value.result.init(std::forward<ResultArgsT>(args)...);
    this->state = SUCCESS_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrickSuccess::complete, this));
    }
}

template<typename... ResultArgsT, typename ReasonT>
bool ValueBrickSuccess<void(ResultArgsT...), ReasonT>::hasResult() const {
    return this->state & SUCCESS_STATE;
}

template<typename... ResultArgsT, typename ReasonT>
std::tuple<ResultArgsT...> & ValueBrickSuccess<void(ResultArgsT...), ReasonT>::getResult() {
    ABB_ASSERT(this->state & SUCCESS_STATE, "Result is not set");
    return *this->value.result;
}

template<typename ResultT, typename ReasonT>
class ValueBrickError {};

template<typename ResultT>
class ValueBrickError<ResultT, Und> : public ValueBrickSuccess<ResultT, Und> {};

template<typename ResultT, typename... ReasonArgsT>
class ValueBrickError<ResultT, void(ReasonArgsT...)> : public ValueBrickSuccess<ResultT, void(ReasonArgsT...)> {
public:
    void setReason(ReasonArgsT... args);

    virtual bool hasReason() const;
    virtual std::tuple<ReasonArgsT...> & getReason();
};

template<typename ResultT, typename... ReasonArgsT>
void ValueBrickError<ResultT, void(ReasonArgsT...)>::setReason(ReasonArgsT... args) {
    ABB_ASSERT(this->state == NEW_STATE, "Already got value");
    this->value.reason.init(std::forward<ReasonArgsT>(args)...);
    this->state = ERROR_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrickError::complete, this));
    }
}

template<typename ResultT, typename... ReasonArgsT>
bool ValueBrickError<ResultT, void(ReasonArgsT...)>::hasReason() const {
    return this->state & ERROR_STATE;
}

template<typename ResultT, typename... ReasonArgsT>
std::tuple<ReasonArgsT...> & ValueBrickError<ResultT, void(ReasonArgsT...)>::getReason() {
    ABB_ASSERT(this->state & ERROR_STATE, "Reason is not set");
    return *this->value.reason;
}

} // namespace internal

template<typename ResultT, typename ReasonT>
using ValueBrick = internal::ValueBrickError<ResultT, ReasonT>;

} // namespace ll
} // namespace abb

#endif // ABB_LL_VALUE_BRICK_H
