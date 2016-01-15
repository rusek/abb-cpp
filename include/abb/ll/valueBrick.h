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

template<typename SuccessorT, typename... ArgsT>
inline void callSuccess(SuccessorT & successor, std::tuple<ArgsT...> result) {
    class OnsuccessCaller {
    public:
        OnsuccessCaller(SuccessorT * successor): successor(successor) {}

        void operator()(ArgsT... args) {
            this->successor->onsuccess(args...);
        }
    private:
        SuccessorT * successor;
    };

    utils::call(OnsuccessCaller(&successor), result);
}

template<typename SuccessorT, typename... ArgsT>
inline void callError(SuccessorT & successor, std::tuple<ArgsT...> reason) {
    class OnerrorCaller {
    public:
        OnerrorCaller(SuccessorT * successor): successor(successor) {}

        void operator()(ArgsT... args) {
            this->successor->onerror(args...);
        }
    private:
        SuccessorT * successor;
    };

    utils::call(OnerrorCaller(&successor), reason);
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

    std::unique_ptr<std::tuple<ResultArgsT...>> resultTuple;
    SuccessorType * successor;
    bool completed;
};

template<typename... ResultArgsT>
ValueBrick<void(ResultArgsT...), Und>::ValueBrick(): resultTuple(), successor(nullptr), completed(false) {}

template<typename... ResultArgsT>
ValueBrick<void(ResultArgsT...), Und>::~ValueBrick() {
    ABB_ASSERT(this->completed, "Not done yet");
}

template<typename... ResultArgsT>
void ValueBrick<void(ResultArgsT...), Und>::setResult(ResultArgsT... args) {
    ABB_ASSERT(!this->resultTuple, "Already got value");
    this->resultTuple.reset(new std::tuple<ResultArgsT...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT>
void ValueBrick<void(ResultArgsT...), Und>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->resultTuple) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT>
void ValueBrick<void(ResultArgsT...), Und>::complete() {
    this->completed = true;
    internal::callSuccess(*this->successor, *this->resultTuple);
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

    std::unique_ptr<std::tuple<ReasonArgsT...>> reasonTuple;
    SuccessorType * successor;
    bool completed;
};

template<typename... ReasonArgsT>
ValueBrick<Und, void(ReasonArgsT...)>::ValueBrick(): reasonTuple(), successor(nullptr), completed(false) {}

template<typename... ReasonArgsT>
ValueBrick<Und, void(ReasonArgsT...)>::~ValueBrick() {
    ABB_ASSERT(this->completed, "Not done yet");
}

template<typename... ReasonArgsT>
void ValueBrick<Und, void(ReasonArgsT...)>::setReason(ReasonArgsT... args) {
    ABB_ASSERT(!this->reasonTuple, "Already got value");
    this->reasonTuple.reset(new std::tuple<ReasonArgsT...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ReasonArgsT>
void ValueBrick<Und, void(ReasonArgsT...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->reasonTuple) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ReasonArgsT>
void ValueBrick<Und, void(ReasonArgsT...)>::complete() {
    this->completed = true;
    internal::callError(*this->successor, *this->reasonTuple);
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

    std::unique_ptr<std::tuple<ResultArgsT...>> resultTuple;
    std::unique_ptr<std::tuple<ReasonArgsT...>> reasonTuple;
    SuccessorType * successor;
    bool completed;
};


template<typename... ResultArgsT, typename... ReasonArgsT>
ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::ValueBrick(): resultTuple(), reasonTuple(), successor(nullptr), completed(false) {}

template<typename... ResultArgsT, typename... ReasonArgsT>
ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::~ValueBrick() {
    ABB_ASSERT(this->completed, "Not done yet");
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::setResult(ResultArgsT... args) {
    ABB_ASSERT(!this->resultTuple && !this->reasonTuple, "Already got value");
    this->resultTuple.reset(new std::tuple<ResultArgsT...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::setReason(ReasonArgsT... args) {
    ABB_ASSERT(!this->resultTuple && !this->reasonTuple, "Already got value");
    this->reasonTuple.reset(new std::tuple<ReasonArgsT...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->resultTuple || this->reasonTuple) {
        Island::current().enqueue(std::bind(&ValueBrick::complete, this));
    }
}

template<typename... ResultArgsT, typename... ReasonArgsT>
void ValueBrick<void(ResultArgsT...), void(ReasonArgsT...)>::complete() {
    this->completed = true;
    if (this->resultTuple) {
        internal::callSuccess(*this->successor, *this->resultTuple);
    } else {
        internal::callError(*this->successor, *this->reasonTuple);
    }
}





} // namespace ll
} // namespace abb

#endif // ABB_LL_VALUE_BRICK_H
