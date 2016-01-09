#ifndef ABB_BLOCK_H
#define ABB_BLOCK_H

#include <abb/ll/brick.h>
#include <abb/ll/proxyBrick.h>
#include <abb/ll/successor.h>

#include <abb/utils/debug.h>

#include <functional>
#include <memory>

namespace abb {

template<typename Done>
class Block {};

template<typename... Args>
class Block<void(Args...)> {
private:
    typedef void FuncType(Args...);
    typedef ll::Brick<void(Args...)> ImplType;
    typedef ll::Successor<void(Args...)> SuccessorType;

public:
    Block(std::unique_ptr<ImplType> impl): impl(std::move(impl)) {}

    bool empty() const {
        return !this->impl;
    }

    template<typename OutBlock>
    OutBlock pipe(std::function<OutBlock(Args...)> func) {
        typedef typename OutBlock::ImplType OutImplType;

        typedef ll::ProxyBrick<typename OutBlock::FuncType> ProxyType;

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

} // namespace abb

#endif // ABB_BLOCK_H
