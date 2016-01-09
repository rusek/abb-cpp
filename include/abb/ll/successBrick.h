#ifndef ABB_LL_SUCCESS_BRICK_H
#define ABB_LL_SUCCESS_BRICK_H

#include <abb/ll/brick.h>

#include <abb/utils/debug.h>
#include <abb/utils/call.h>

#include <memory>
#include <tuple>

namespace abb {
namespace ll {

template<typename Done>
class SuccessBrick {};

template<typename... Args>
class SuccessBrick<void(Args...)> : public Brick<void(Args...)> {
public:
    typedef SuccessBrick<void(Args...)> ThisType;
    typedef Successor<void(Args...)> SuccessorType;

    SuccessBrick(): argsTuple(), successor(nullptr), notified(false) {}

    virtual ~SuccessBrick() {
        ABB_ASSERT(this->notified, "Not done yet");
    }

    void setResult(Args... args) {
        ABB_ASSERT(!this->argsTuple, "Already got result");
        this->argsTuple.reset(new std::tuple<Args...>(args...));
        this->tryFinish();
    }

    virtual void setSuccessor(SuccessorType & successor) {
        ABB_ASSERT(!this->successor, "Already got successor");
        this->successor = &successor;
        this->tryFinish();
    }

    static ThisType * newWithResult(Args... args) {
        ThisType * block = new ThisType();
        block->setResult(args...);
        return block;
    }

private:
    void run() {
        class DoneCaller {
        public:
            DoneCaller(SuccessorType * successor): successor(successor) {}

            void operator()(Args... args) {
                this->successor->done(args...);
            }
        private:
            SuccessorType * successor;
        };

        std::unique_ptr<std::tuple<Args...>> argsTuple(std::move(this->argsTuple));
        this->notified = true;
        utils::call(DoneCaller(this->successor), *argsTuple);
    }

    void tryFinish() {
        if (this->argsTuple && this->successor) {
            Island::current().enqueue(std::bind(&SuccessBrick::run, this));
        }
    }

    std::unique_ptr<std::tuple<Args...>> argsTuple;
    SuccessorType * successor;
    bool notified;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_SUCCESS_BRICK_H
