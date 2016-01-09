#ifndef ABB_H
#define ABB_H

#include <tuple>
#include <deque>
#include <functional>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <type_traits>

#define ABB_FIASCO(msg) \
    do { \
        std::cerr << "File " << __FILE__ << ", line " << __LINE__ << ": " << msg << std::endl; \
        std::abort(); \
    } while (0)

#define ABB_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            ABB_FIASCO(msg); \
        } \
    } while (0)

namespace abb {

namespace detail
{
    template <typename F, typename Tuple, bool Done, int Total, int... N>
    struct call_impl
    {
        static void call(F f, Tuple t)
        {
            call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, t);
        }
    };

    template <typename F, typename Tuple, int Total, int... N>
    struct call_impl<F, Tuple, true, Total, N...>
    {
        static void call(F f, Tuple t)
        {
            f(std::get<N>(t)...);
        }
    };
}

// user invokes this
template <typename F, typename Tuple>
void call(F f, Tuple t)
{
    typedef typename std::decay<Tuple>::type ttype;
    detail::call_impl<F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, Tuple(t));
}

class Noncopyable {
public:
    Noncopyable() {}
private:
    Noncopyable(Noncopyable const&);
    void operator=(Noncopyable const&);
};

class Island {
public:
    typedef std::function<void()> Task;

    void enqueue(Task task) {
        this->tasks.push_back(task);
    }

    void run() {
        ABB_ASSERT(Island::currentPtr == nullptr, "Current island already set");
        Island::currentPtr = this;

        while (!this->tasks.empty()) {
            Task task = this->tasks.front();
            this->tasks.pop_front();
            task();
        }

        ABB_ASSERT(Island::currentPtr == this, "Current island changed in the meantime");
        Island::currentPtr = nullptr;
    }

    static Island & current() {
        ABB_ASSERT(Island::currentPtr != nullptr, "Current island not set");
        return *Island::currentPtr;
    }

private:
    std::deque<Task> tasks;

    static Island * currentPtr;
};

Island * Island::currentPtr = nullptr;


template<typename DoneCont>
class Successor {
public:
};

template<typename... Args>
class Successor<void(Args...)> {
public:
    virtual ~Successor() {}

    virtual void done(Args...) = 0;
};


template<typename DoneCont>
class BlockImpl {};

template<typename... Args>
class BlockImpl<void(Args...)> : Noncopyable {
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
        call(DoneCaller(this->successor), *argsTuple);
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



template<typename Done>
class Cont {

};

template<typename... Args>
Block<void(Args...)> success(Args... args) {
    return std::unique_ptr<BlockImpl<void(Args...)>>(SuccessBlock<void(Args...)>::newWithResult(args...));
}

} // namespace abb

#endif // ABB_H