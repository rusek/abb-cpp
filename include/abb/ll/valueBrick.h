#ifndef ABB_LL_VALUE_BRICK_H
#define ABB_LL_VALUE_BRICK_H

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

template<typename ResultT, typename ResultFuncT, typename ReasonT, typename ReasonFuncT>
class ValueBrick {};

template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
class ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)> : public Brick<ResultT, ReasonT> {
public:
    typedef Brick<ResultT, ReasonT> BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

    ValueBrick();

    virtual ~ValueBrick();

    void setResult(ResultArgsT... args);
    void setReason(ReasonArgsT... args);

    virtual void setSuccessor(SuccessorType & successor);

private:
    typedef IsDefined<ResultT> ResultDefined;
    typedef IsDefined<ReasonT> ReasonDefined;

    void completeSuccess(std::true_type) {
        class OnsuccessCaller {
        public:
            OnsuccessCaller(SuccessorType * successor): successor(successor) {}

            void operator()(ResultArgsT... args) {
                this->successor->onsuccess(args...);
            }
        private:
            SuccessorType * successor;
        };

        std::unique_ptr<std::tuple<ResultArgsT...>> resultTuple(std::move(this->resultTuple));
        this->completed = true;
        utils::call(OnsuccessCaller(this->successor), *resultTuple);
    }

    void completeSuccess(std::false_type) {}

    void completeError(std::true_type) {
        class OnerrorCaller {
        public:
            OnerrorCaller(SuccessorType * successor): successor(successor) {}

            void operator()(ReasonArgsT... args) {
                this->successor->onerror(args...);
            }
        private:
            SuccessorType * successor;
        };

        if (this->reasonTuple) {
            std::unique_ptr<std::tuple<ReasonArgsT...>> reasonTuple(std::move(this->reasonTuple));
            this->completed = true;
            utils::call(OnerrorCaller(this->successor), *reasonTuple);
        }
    }

    void completeError(std::false_type) {}

    void complete();

    std::unique_ptr<std::tuple<ResultArgsT...>> resultTuple;
    std::unique_ptr<std::tuple<ReasonArgsT...>> reasonTuple;
    SuccessorType * successor;
    bool completed;
};


template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)>::ValueBrick(): resultTuple(), reasonTuple(), successor(nullptr), completed(false) {}

template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)>::~ValueBrick() {
    ABB_ASSERT(this->completed, "Not done yet");
}

template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
void ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)>::setResult(ResultArgsT... args) {
    static_assert(ResultDefined::value, "ResultT must be defined");

    ABB_ASSERT(!this->resultTuple && !this->reasonTuple, "Already got value");
    this->resultTuple.reset(new std::tuple<ResultArgsT...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
void ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)>::setReason(ReasonArgsT... args) {
    static_assert(ReasonDefined::value, "ReasonT must be defined");

    ABB_ASSERT(!this->resultTuple && !this->reasonTuple, "Already got value");
    this->reasonTuple.reset(new std::tuple<ReasonArgsT...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
void ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->resultTuple || this->reasonTuple) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename ResultT, typename... ResultArgsT, typename ReasonT, typename... ReasonArgsT>
void ValueBrick<ResultT, void(ResultArgsT...), ReasonT, void(ReasonArgsT...)>::complete() {
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

template<typename ResultT, typename ReasonT>
class ValueBrick : public internal::ValueBrick<
    ResultT,
    typename internal::AsFunc<ResultT>::Type,
    ReasonT,
    typename internal::AsFunc<ReasonT>::Type
> {};


} // namespace ll
} // namespace abb

#endif // ABB_LL_VALUE_BRICK_H

