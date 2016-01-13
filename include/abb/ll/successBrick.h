#ifndef ABB_LL_SUCCESS_BRICK_H
#define ABB_LL_SUCCESS_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>

#include <memory>
#include <tuple>

namespace abb {
namespace ll {

namespace internal {

template<typename Value>
struct IsDefined : std::true_type {};

template<>
struct IsDefined<Und> : std::false_type {};

template<typename Result, typename ResultFunc, typename Reason, typename ReasonFunc>
class ValueBrick {};

template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
class ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)> : public Brick<Result, Reason> {
public:
    typedef Brick<Result, Reason> BaseType;
    typedef Successor<Result, Reason> SuccessorType;

    ValueBrick();

    virtual ~ValueBrick();

    void setResult(ResultArgs... args);
    void setReason(ReasonArgs... args);

    virtual void setSuccessor(SuccessorType & successor);

private:
    typedef IsDefined<Result> ResultDefined;
    typedef IsDefined<Reason> ReasonDefined;

    void completeSuccess(std::true_type) {
        class OnsuccessCaller {
        public:
            OnsuccessCaller(SuccessorType * successor): successor(successor) {}

            void operator()(ResultArgs... args) {
                this->successor->onsuccess(args...);
            }
        private:
            SuccessorType * successor;
        };

        std::unique_ptr<std::tuple<ResultArgs...>> resultTuple(std::move(this->resultTuple));
        this->completed = true;
        utils::call(OnsuccessCaller(this->successor), *resultTuple);
    }

    void completeSuccess(std::false_type) {}

    void completeError(std::true_type) {
        class OnerrorCaller {
        public:
            OnerrorCaller(SuccessorType * successor): successor(successor) {}

            void operator()(ReasonArgs... args) {
                this->successor->onerror(args...);
            }
        private:
            SuccessorType * successor;
        };

        if (this->reasonTuple) {
            std::unique_ptr<std::tuple<ReasonArgs...>> reasonTuple(std::move(this->reasonTuple));
            this->completed = true;
            utils::call(OnerrorCaller(this->successor), *reasonTuple);
        }
    }

    void completeError(std::false_type) {}

    void complete();

    std::unique_ptr<std::tuple<ResultArgs...>> resultTuple;
    std::unique_ptr<std::tuple<ReasonArgs...>> reasonTuple;
    SuccessorType * successor;
    bool completed;
};


template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)>::ValueBrick(): resultTuple(), reasonTuple(), successor(nullptr), completed(false) {}

template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)>::~ValueBrick() {
    ABB_ASSERT(this->completed, "Not done yet");
}

template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
void ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)>::setResult(ResultArgs... args) {
    static_assert(ResultDefined::value, "Result must be defined");

    ABB_ASSERT(!this->resultTuple && !this->reasonTuple, "Already got value");
    this->resultTuple.reset(new std::tuple<ResultArgs...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
void ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)>::setReason(ReasonArgs... args) {
    static_assert(ReasonDefined::value, "Reason must be defined");

    ABB_ASSERT(!this->resultTuple && !this->reasonTuple, "Already got value");
    this->reasonTuple.reset(new std::tuple<ReasonArgs...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
void ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->resultTuple || this->reasonTuple) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename Result, typename... ResultArgs, typename Reason, typename... ReasonArgs>
void ValueBrick<Result, void(ResultArgs...), Reason, void(ReasonArgs...)>::complete() {
    if (ResultDefined::value && (!ReasonDefined::value || this->resultTuple)) {
        this->completeSuccess(ResultDefined());
    } else {
        this->completeError(ReasonDefined());
    }
}

template<typename Value>
struct AsFunc {};

template<typename... Args>
struct AsFunc<void(Args...)> {
    typedef void Type(Args...);
};

template<>
struct AsFunc<Und> {
    typedef void Type();
};

} // namespace internal

template<typename Result, typename Reason>
class ValueBrick : public internal::ValueBrick<
    Result,
    typename internal::AsFunc<Result>::Type,
    Reason,
    typename internal::AsFunc<Reason>::Type
> {};


} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H

