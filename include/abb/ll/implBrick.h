#ifndef ABB_LL_IMPL_BRICK_H
#define ABB_LL_IMPL_BRICK_H

#include <abb/ll/brick.h>

#include <abb/island.h>
#include <abb/reply.h>

#include <abb/utils/debug.h>
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

template<typename ResultT, typename ReasonT, typename FuncT>
class ImplBrick : public Brick<ResultT, ReasonT>, protected Task, protected Reply<ResultT, ReasonT> {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    template<typename ArgT>
    ImplBrick(ArgT && arg): func(std::forward<ArgT>(arg)), status(PENDING), successor(nullptr) {}

    ~ImplBrick() {
        if (this->status & (SUCCESS | ERROR)) {
            this->value.destroy(this->status & SUCCESS);
        }
    }

    void abort() {}

    void start(Successor & successor) {
        ABB_ASSERT(!this->successor, "Already got successor");
        this->successor = &successor;
        this->func(*static_cast<Reply<ResultT, ReasonT>*>(this));
    }

    Status getStatus() const {
        return this->status;
    }

    ValueToTuple<ResultT> & getResult() {
        ABB_ASSERT(this->status & SUCCESS, "Result is not set");
        return *this->value.result;
    }

    ValueToTuple<ReasonT> & getReason() {
        ABB_ASSERT(this->status & ERROR, "Reason is not set");
        return *this->value.reason;
    }

protected:
    virtual void run();

    virtual Island & getIsland() const;
    virtual bool isAborted() const;
    virtual void setAborted();

    FuncT func;
    internal::ValueToBank<ResultType, ReasonType> value;
    Status status;
    Successor * successor;
};

template<typename ResultT, typename ReasonT, typename FuncT>
void ImplBrick<ResultT, ReasonT, FuncT>::run() {
    this->successor->onUpdate();
}

template<typename ResultT, typename ReasonT, typename FuncT>
Island & ImplBrick<ResultT, ReasonT, FuncT>::getIsland() const {
    ABB_ASSERT(this->status == PENDING, "Cannot call getIsland after setting a value");
    return this->successor->getIsland();
}

template<typename ResultT, typename ReasonT, typename FuncT>
bool ImplBrick<ResultT, ReasonT, FuncT>::isAborted() const {
    ABB_ASSERT(this->status == PENDING, "Cannot call getIsland after setting a value");
    return this->successor->isAborted();
}

template<typename ResultT, typename ReasonT, typename FuncT>
void ImplBrick<ResultT, ReasonT, FuncT>::setAborted() {
    ABB_ASSERT(this->status == PENDING, "Already got value");
    ABB_ASSERT(this->successor->isAborted(), "Abort was not requested");
    this->status = ABORT;
    this->successor->getIsland().enqueue(static_cast<Task&>(*this));
}

template<typename ResultT, typename ReasonT, typename FuncT>
class ImplBrickSuccess : public ImplBrick<ResultT, ReasonT, FuncT> {
public:
    using ImplBrick<ResultT, ReasonT, FuncT>::ImplBrick;
};

template<typename... ArgsT, typename ReasonT, typename FuncT>
class ImplBrickSuccess<void(ArgsT...), ReasonT, FuncT> : public ImplBrick<void(ArgsT...), ReasonT, FuncT> {
public:
    using ImplBrick<void(ArgsT...), ReasonT, FuncT>::ImplBrick;

protected:
    virtual void setResult(ArgsT... args) {
        ABB_ASSERT(this->status == PENDING, "Already got value");
        this->value.result.init(std::move(args)...);
        this->status = SUCCESS;
        this->successor->getIsland().enqueue(static_cast<Task&>(*this));
    }
};

template<typename ResultT, typename ReasonT, typename FuncT>
class ImplBrickError : public ImplBrickSuccess<ResultT, ReasonT, FuncT> {
public:
    using ImplBrickSuccess<ResultT, ReasonT, FuncT>::ImplBrickSuccess;
};

template<typename ResultT, typename... ArgsT, typename FuncT>
class ImplBrickError<ResultT, void(ArgsT...), FuncT> : public ImplBrickSuccess<ResultT, void(ArgsT...), FuncT> {
public:
    using ImplBrickSuccess<ResultT, void(ArgsT...), FuncT>::ImplBrickSuccess;

protected:
    virtual void setReason(ArgsT... args) {
        ABB_ASSERT(this->status == PENDING, "Already got value");
        this->value.reason.init(std::move(args)...);
        this->status = ERROR;
        this->successor->getIsland().enqueue(*this);
    }
};

} // namespace internal

template<typename ResultT, typename ReasonT, typename FuncT>
class ImplBrick : public internal::ImplBrickError<ResultT, ReasonT, FuncT> {
public:
    using internal::ImplBrickError<ResultT, ReasonT, FuncT>::ImplBrickError;
};

} // namespace ll
} // namespace abb

#endif // ABB_LL_IMPL_BRICK_H
