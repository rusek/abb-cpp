#ifndef ABB_LL_BRICK_PTR_H
#define ABB_LL_BRICK_PTR_H

#include <abb/ll/brick.h>
#include <memory>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class BrickPtr {
public:
    typedef Brick<ResultT, ReasonT> BrickType;

    BrickPtr() {}
    explicit BrickPtr(BrickType * raw): ptr(raw) {}

    explicit operator bool() const {
        return bool(this->ptr);
    }

    void setSuccessor(Successor & successor) {
        this->ptr->setSuccessor(successor);
    }

    bool hasResult() const {
        return this->ptr->hasResult();
    }

    ValueToTuple<ResultT> & getResult() {
        return this->ptr->getResult();
    }

    bool hasReason() const {
        return this->ptr->hasReason();
    }

    ValueToTuple<ReasonT> & getReason() {
        return this->ptr->getReason();
    }

private:
    std::unique_ptr<BrickType> ptr;
};

namespace internal {

template<typename BrickT>
using BrickToPtr = BrickPtr<typename BrickT::ResultType, typename BrickT::ReasonType>;

} // namespace internal

template<typename BrickT>
inline internal::BrickToPtr<BrickT> makeBrickPtr(BrickT * raw) {
    return internal::BrickToPtr<BrickT>(raw);
}

template<typename BrickT, typename... ArgsT>
inline internal::BrickToPtr<BrickT> makeBrick(ArgsT &&... args) {
    return internal::BrickToPtr<BrickT>(new BrickT(std::forward<ArgsT>(args)...));
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_PTR_H
