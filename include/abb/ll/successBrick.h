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

template<typename Result>
class SuccessBrick {};

template<typename... Args>
class SuccessBrick<void(Args...)> : public Brick<void(Args...)> {
public:
    typedef Brick<void(Args...)> BaseType;
    typedef Successor<void(Args...)> SuccessorType;

    SuccessBrick();

    virtual ~SuccessBrick();

    void setResult(Args... args);

    virtual void setSuccessor(SuccessorType & successor);

private:
    void complete();

    std::unique_ptr<std::tuple<Args...>> argsTuple;
    SuccessorType * successor;
    bool completed;
};


template<typename... Args>
SuccessBrick<void(Args...)>::SuccessBrick(): argsTuple(), successor(nullptr), completed(false) {}

template<typename... Args>
SuccessBrick<void(Args...)>::~SuccessBrick() {
    ABB_ASSERT(this->completed, "Not done yet");
}

template<typename... Args>
void SuccessBrick<void(Args...)>::setResult(Args... args) {
    ABB_ASSERT(!this->argsTuple, "Already got result");
    this->argsTuple.reset(new std::tuple<Args...>(args...));
    if (this->successor) {
        Island::current().enqueue(std::bind(&SuccessBrick::complete, this));
    }
}

template<typename... Args>
void SuccessBrick<void(Args...)>::setSuccessor(SuccessorType & successor) {
    ABB_ASSERT(!this->successor, "Already got successor");
    this->successor = &successor;
    if (this->argsTuple) {
        Island::current().enqueue(std::bind(&SuccessBrick::complete, this));
    }
}

template<typename... Args>
void SuccessBrick<void(Args...)>::complete() {
    class OnsuccessCaller {
    public:
        OnsuccessCaller(SuccessorType * successor): successor(successor) {}

        void operator()(Args... args) {
            this->successor->onsuccess(args...);
        }
    private:
        SuccessorType * successor;
    };

    std::unique_ptr<std::tuple<Args...>> argsTuple(std::move(this->argsTuple));
    this->completed = true;
    utils::call(OnsuccessCaller(this->successor), *argsTuple);
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H
