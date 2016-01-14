#ifndef ABB_LL_DETACH_SUCCESSOR_H
#define ABB_LL_DETACH_SUCCESSOR_H

#include <abb/ll/successor.h>
#include <abb/ll/brick.h>

#include <abb/und.h>

#include <memory>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class DetachSuccessor {};

template<typename... ArgsT>
class DetachSuccessor<void(ArgsT...), Und> : public Successor<void(ArgsT...), Und> {
public:
    DetachSuccessor(std::unique_ptr<ll::Brick<void(ArgsT...), Und>> brick): brick(std::move(brick)) {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(ArgsT...) {
        delete this;
    }
private:
    std::unique_ptr<Brick<void(ArgsT...), Und>> brick;
};

template<typename... ArgsT>
class DetachSuccessor<Und, void(ArgsT...)> : public Successor<Und, void(ArgsT...)> {
public:
    DetachSuccessor(std::unique_ptr<Brick<Und, void(ArgsT...)>> brick): brick(std::move(brick)) {
        this->brick->setSuccessor(*this);
    }

    virtual void onerror(ArgsT...) {
        delete this;
    }
private:
    std::unique_ptr<Brick<Und, void(ArgsT...)>> brick;
};

template<typename... ResultArgsT, typename... ReasonArgsT>
class DetachSuccessor<void(ResultArgsT...), void(ReasonArgsT...)> : public Successor<void(ResultArgsT...), void(ReasonArgsT...)> {
public:
    DetachSuccessor(std::unique_ptr<Brick<void(ResultArgsT...), void(ReasonArgsT...)>> brick): brick(std::move(brick)) {
        this->brick->setSuccessor(*this);
    }

    virtual void onsuccess(ResultArgsT...) {
        delete this;
    }

    virtual void onerror(ReasonArgsT...) {
        delete this;
    }
private:
    std::unique_ptr<Brick<void(ResultArgsT...), void(ReasonArgsT...)>> brick;
};


} // namespace ll
} // namespace abb

#endif // ABB_LL_DETACH_SUCCESSOR_H
