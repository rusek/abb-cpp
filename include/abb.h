#ifndef ABB_H
#define ABB_H

#include <abb/utils/call.h>
#include <abb/utils/debug.h>
#include <abb/utils/noncopyable.h>
#include <abb/island.h>
#include <abb/successor.h>

#include <tuple>
#include <deque>
#include <functional>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <type_traits>

namespace abb {

template<typename DoneCont>
class BlockImpl {};

template<typename... Args>
class BlockImpl<void(Args...)> : utils::Noncopyable {
public:
    virtual void setSuccessor(Successor<void(Args...)> & successor) = 0;

    virtual ~BlockImpl() {}
};


template<typename Done>
class SuccessBlock {};

template<typename... Args>
class SuccessBlock<void(Args...)> : public BlockImpl<void(Args...)> {
public:
    typedef SuccessBlock<void(Args...)> ThisType;
    typedef Successor<void(Args...)> SuccessorType;

    SuccessBlock(): argsTuple(), successor(nullptr), notified(false) {}

    virtual ~SuccessBlock() {
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
            Island::current().enqueue(std::bind(&SuccessBlock::run, this));
        }
    }

    std::unique_ptr<std::tuple<Args...>> argsTuple;
    SuccessorType * successor;
    bool notified;
};

template<typename Done>
class ProxyBlock {};

template<typename... Args>
class ProxyBlock<void(Args...)> : public BlockImpl<void(Args...)> {
public:
    typedef ProxyBlock<void(Args...)> ThisType;
    typedef BlockImpl<void(Args...)> ImplType;
    typedef Successor<void(Args...)> SuccessorType;

    ProxyBlock(): block(), successor(nullptr) {}

    virtual ~ProxyBlock() {
        ABB_ASSERT(this->block && this->successor, "Not done yet");
    }

    void setBlock(std::unique_ptr<ImplType> block) {
        ABB_ASSERT(!this->block, "Already got block");
        this->block = std::move(block);
        this->tryFinish();
    }

    virtual void setSuccessor(SuccessorType & successor) {
        ABB_ASSERT(!this->successor, "Already got successor");
        this->successor = &successor;
        this->tryFinish();
    }

private:
    void tryFinish() {
        if (this->block && this->successor) {
            this->block->setSuccessor(*this->successor);
        }
    }

    std::unique_ptr<ImplType> block;
    SuccessorType * successor;
};

template<typename Done>
class Block {};

template<typename... Args>
class Block<void(Args...)> {
private:
    typedef void FuncType(Args...);
    typedef BlockImpl<void(Args...)> ImplType;
    typedef Successor<void(Args...)> SuccessorType;

public:
    Block(std::unique_ptr<ImplType> impl): impl(std::move(impl)) {}

    bool empty() const {
        return !this->impl;
    }

    template<typename OutBlock>
    OutBlock pipe(std::function<OutBlock(Args...)> func) {
        typedef typename OutBlock::ImplType OutImplType;

        typedef ProxyBlock<typename OutBlock::FuncType> ProxyType;

        class LeftSuccessor : public SuccessorType {
        public:
            LeftSuccessor(
                std::unique_ptr<ImplType> impl,
                std::function<OutBlock(Args...)> cont,
                ProxyType & proxy
            ):
                impl(std::move(impl)),
                cont(cont),
                proxy(proxy)
            {
                this->impl->setSuccessor(*this);
            }

            virtual void done(Args... args) {
                this->proxy.setBlock(std::move(this->cont(args...).impl));
                delete this;
            }

        private:
            std::unique_ptr<ImplType> impl;
            std::function<OutBlock(Args...)> cont;
            ProxyType & proxy;
        };

        ABB_ASSERT(this->impl, "Block is empty");

        ProxyType * pipeBlock = new ProxyType();

        new LeftSuccessor(std::move(this->impl), func, *pipeBlock);

        return std::unique_ptr<OutImplType>(pipeBlock);
    }

    void detach() {
        class ImplSuccessor : public SuccessorType {
        public:
            ImplSuccessor(std::unique_ptr<ImplType> impl): impl(std::move(impl)) {
                this->impl->setSuccessor(*this);
            }

            virtual void done(Args... args) {
                delete this;
            }
        private:
            std::unique_ptr<ImplType> impl;
        };

        if (this->impl) {
            new ImplSuccessor(std::move(this->impl));
        }
    }

private:
    std::unique_ptr<ImplType> impl;
};

template<typename... Args>
Block<void(Args...)> success(Args... args) {
    return std::unique_ptr<BlockImpl<void(Args...)>>(SuccessBlock<void(Args...)>::newWithResult(args...));
}

} // namespace abb

#endif // ABB_H