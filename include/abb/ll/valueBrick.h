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
    DONE_STATE = 4
};

template<typename SuccessorT, typename... ArgsT>
inline void callSuccess(SuccessorT & successor, std::tuple<ArgsT...> && result) {
    utils::call(
        [&](ArgsT &&... args) { successor.onsuccess(std::forward<ArgsT>(args)...); },
        std::forward<std::tuple<ArgsT...>>(result)
    );
}

template<typename SuccessorT, typename... ArgsT>
inline void callError(SuccessorT & successor, std::tuple<ArgsT...> && reason) {
    utils::call(
        [&](ArgsT &&... args) { successor.onerror(std::forward<ArgsT>(args)...); },
        std::forward<std::tuple<ArgsT...>>(reason)
    );
}

} // namespace internal

template<typename ResultT, typename ReasonT>
class ValueBrick {};


template<typename... ResultArgsT>
class ValueBrick<void(ResultArgsT...), Und> : public Brick<void(ResultArgsT...), Und> {
public:
    typedef Brick<void(ResultArgsT...), Und> BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

    ValueBrick();

    virtual ~ValueBrick();

    void setResult(ResultArgsT... args);

    virtual void setSuccessor(SuccessorType & successor);

private:
    void complete();

    utils::Vault<std::tuple<ResultArgsT...>> result;
    internal::State state;
    SuccessorType * successor;
};

template<typename... ResultArgsT>
ValueBrick<void(ResultArgsT...), Und>::ValueBrick(): state(internal::NEW_STATE), successor(nullptr) {}

template<typename... ResultArgsT>
ValueBrick<void(ResultArgsT...), Und>::~ValueBrick() {
    ABB_ASSERT(this->state & internal::DONE_STATE, "Not done yet");
    this->result.destroy();
}

template<typename... ResultArgsT>
void ValueBrick<void(ResultArgsT...), Und>::setResult(ResultArgsT... args) {
    ABB_ASSERT(this->state == internal::NEW_STATE, "Already got value");
    this->result.init(std::forward<ResultArgsT>(args)...);
    this->state = internal::SUCCESS_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT>
void ValueBrick<void(ResultArgsT...), Und>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->state != internal::NEW_STATE) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT>
void ValueBrick<void(ResultArgsT...), Und>::complete() {
    this->state |= internal::DONE_STATE;
    internal::callSuccess(*this->successor, std::move(*this->result));
}

template<typename... ReasonArgsT>
class ValueBrick<Und, void(ReasonArgsT...)> : public Brick<Und, void(ReasonArgsT...)> {
public:
    typedef Brick<Und, void(ReasonArgsT...)> BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

    ValueBrick();

    virtual ~ValueBrick();

    void setReason(ReasonArgsT... args);

    virtual void setSuccessor(SuccessorType & successor);

private:
    void complete();

    utils::Vault<std::tuple<ReasonArgsT...>> reason;
    internal::State state;
    SuccessorType * successor;
};

template<typename... ReasonArgsT>
ValueBrick<Und, void(ReasonArgsT...)>::ValueBrick(): state(internal::NEW_STATE), successor(nullptr) {}

template<typename... ReasonArgsT>
ValueBrick<Und, void(ReasonArgsT...)>::~ValueBrick() {
    ABB_ASSERT(this->state & internal::DONE_STATE, "Not done yet");
    this->reason.destroy();
}

template<typename... ReasonArgsT>
void ValueBrick<Und, void(ReasonArgsT...)>::setReason(ReasonArgsT... args) {
    ABB_ASSERT(this->state == internal::NEW_STATE, "Already got value");
    this->reason.init(std::forward<ReasonArgsT>(args)...);
    this->state = internal::ERROR_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ReasonArgsT>
void ValueBrick<Und, void(ReasonArgsT...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->state != internal::NEW_STATE) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ReasonArgsT>
void ValueBrick<Und, void(ReasonArgsT...)>::complete() {
    this->state |= internal::DONE_STATE;
    internal::callError(*this->successor, std::move(*this->reason));
}


template<typename... ResultArgsT, typename... ReasonArgsT>
class ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)> : public Brick<void(ResultArgsT...), void(ReasonArgsT...)> {
public:
    typedef Brick<void(ResultArgsT...), void(ReasonArgsT...)> BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

    ValueBrick();

    virtual ~ValueBrick();

    void setResult(ResultArgsT... args);
    void setReason(ReasonArgsT... args);

    virtual void setSuccessor(SuccessorType & successor);

private:
    void complete();

    union {
        utils::Vault<std::tuple<ResultArgsT...>> result;
        utils::Vault<std::tuple<ReasonArgsT...>> reason;
    } value;
    internal::State state;
    SuccessorType * successor;
};


template<typename... ResultArgsT, typename... ReasonArgsT>
ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::ValueBrick(): state(internal::NEW_STATE), successor(nullptr) {}

template<typename... ResultArgsT, typename... ReasonArgsT>
ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::~ValueBrick() {
    ABB_ASSERT(this->state & internal::DONE_STATE, "Not done yet");
    if (this->state & internal::SUCCESS_STATE) {
        this->value.result.destroy();
    } else {
        this->value.reason.destroy();
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::setResult(ResultArgsT... args) {
    ABB_ASSERT(this->state == internal::NEW_STATE, "Already got value");
    this->value.result.init(std::forward<ResultArgsT>(args)...);
    this->state = internal::SUCCESS_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::setReason(ReasonArgsT... args) {
    ABB_ASSERT(this->state == internal::NEW_STATE, "Already got value");
    this->value.reason.init(std::forward<ReasonArgsT>(args)...);
    this->state = internal::ERROR_STATE;
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->state != internal::NEW_STATE) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::complete() {
    this->state |= internal::DONE_STATE;
    if (this->state & internal::SUCCESS_STATE) {
        internal::callSuccess(*this->successor, std::move(*this->value.result));
    } else {
        internal::callError(*this->successor, std::move(*this->value.reason));
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_VALUE_BRICK_H
