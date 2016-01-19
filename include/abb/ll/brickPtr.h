#ifndef ABB_LL_BRICK_PTR_H
#define ABB_LL_BRICK_PTR_H

#include <abb/ll/brick.h>
#include <memory>

namespace abb {
namespace ll {

namespace internal {

typedef void RawBrick;
typedef void RawValue;

struct BrickVtable {
    // Base methods
    void (*setSuccessor)(RawBrick * brick, Successor & successor);
    void (*destroy)(RawBrick * brick);
    // Success-related methods
    bool (*hasResult)(RawBrick const* brick);
    RawValue * (*getResult)(RawBrick * brick);
    // Error-related methods
    bool (*hasReason)(RawBrick const* brick);
    RawValue * (*getReason)(RawBrick * brick);
};


template<typename ValueT>
struct ValueFuncs {
    static RawValue * toRaw(ValueToTuple<ValueT> * value) {
        return static_cast<RawValue*>(value);
    }

    static ValueToTuple<ValueT> * fromRaw(RawValue * value) {
        return static_cast<ValueToTuple<ValueT>*>(value);
    }
};

template<typename BrickT>
struct BrickFuncs {
    static RawBrick * toRaw(BrickT * brick) {
        return static_cast<RawBrick*>(brick);
    }

    static void setSuccessor(RawBrick * brick, Successor & successor);
    static void destroy(RawBrick * brick);
    static bool hasResult(RawBrick const* brick);
    static RawValue * getResult(RawBrick * brick);
    static bool hasReason(RawBrick const* brick);
    static RawValue * getReason(RawBrick * brick);

    static const BrickVtable vtable;

private:
    typedef IsDefined<typename BrickT::ResultType> ResultDefined;
    typedef IsDefined<typename BrickT::ReasonType> ReasonDefined;

    static BrickT * fromRaw(RawBrick * brick) {
        return static_cast<BrickT*>(brick);
    }

    static BrickT const* fromRaw(RawBrick const* brick) {
        return static_cast<BrickT const*>(brick);
    }

    static bool hasResult(RawBrick const* brick, std::true_type) {
        return BrickFuncs::fromRaw(brick)->hasResult();
    }
    static bool hasResult(RawBrick const*, std::false_type) {
        return false;
    }

    static RawValue * getResult(RawBrick * brick, std::true_type) {
        return ValueFuncs<typename BrickT::ResultType>::toRaw(&BrickFuncs::fromRaw(brick)->getResult());
    }
    static RawValue * getResult(RawBrick *, std::false_type) {
        ABB_FIASCO("Erased method called");
    }

    static bool hasReason(RawBrick const* brick, std::true_type) {
        return BrickFuncs::fromRaw(brick)->hasReason();
    }
    static bool hasReason(RawBrick const*, std::false_type) {
        return false;
    }

    static RawValue * getReason(RawBrick * brick, std::true_type) {
        return ValueFuncs<typename BrickT::ReasonType>::toRaw(&BrickFuncs::fromRaw(brick)->getReason());
    }
    static RawValue * getReason(RawBrick *, std::false_type) {
        ABB_FIASCO("Erased method called");
    }
};

template<typename BrickT>
void BrickFuncs<BrickT>::destroy(RawBrick * brick) {
    delete BrickFuncs::fromRaw(brick);
}

template<typename BrickT>
void BrickFuncs<BrickT>::setSuccessor(RawBrick * brick, Successor & successor) {
    BrickFuncs::fromRaw(brick)->setSuccessor(successor);
}

template<typename BrickT>
bool BrickFuncs<BrickT>::hasResult(RawBrick const* brick) {
    return BrickFuncs::hasResult(brick, ResultDefined());
}

template<typename BrickT>
RawValue * BrickFuncs<BrickT>::getResult(RawBrick * brick) {
    return BrickFuncs::getResult(brick, ResultDefined());
}

template<typename BrickT>
bool BrickFuncs<BrickT>::hasReason(RawBrick const* brick) {
    return BrickFuncs::hasReason(brick, ReasonDefined());
}

template<typename BrickT>
RawValue * BrickFuncs<BrickT>::getReason(RawBrick * brick) {
    return BrickFuncs::getReason(brick, ReasonDefined());
}

template<typename BrickT>
const BrickVtable BrickFuncs<BrickT>::vtable = {
    &BrickFuncs::setSuccessor,
    &BrickFuncs::destroy,
    &BrickFuncs::hasResult,
    &BrickFuncs::getResult,
    &BrickFuncs::hasReason,
    &BrickFuncs::getReason
};

template<typename ValueT, typename OtherValueT>
struct IsValueSubstitutable : std::integral_constant<
    bool,
    std::is_same<ValueT, OtherValueT>::value ||
        std::is_same<OtherValueT, Und>::value
> {};

} // namespace internal

template<typename ResultT, typename ReasonT>
class BrickPtr {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    BrickPtr(): vtable(nullptr), ptr(nullptr) {}

    template<typename BrickT>
    explicit BrickPtr(BrickT * brick):
        vtable(&internal::BrickFuncs<BrickT>::vtable),
        ptr(internal::BrickFuncs<BrickT>::toRaw(brick)) {}

    BrickPtr(BrickPtr const&) = delete;

    template<
        typename OtherResultT,
        typename OtherReasonT,
        typename std::enable_if<
            internal::IsValueSubstitutable<ResultT, OtherResultT>::value &&
            internal::IsValueSubstitutable<ReasonT, OtherReasonT>::value
        >::type* = nullptr
    > BrickPtr(BrickPtr<OtherResultT, OtherReasonT> && other):
        vtable(other.vtable),
        ptr(other.ptr)
    {
        other.vtable = nullptr;
        other.ptr = nullptr;
    }

    BrickPtr & operator=(BrickPtr const&) = delete;
    BrickPtr & operator=(BrickPtr && other) {
        if (this->ptr) {
            this->vtable->destroy(this->ptr);
        }
        this->vtable = other.vtable;
        this->ptr = other.ptr;
        other.vtable = nullptr;
        other.ptr = nullptr;
        return *this;
    }

    ~BrickPtr() {
        if (this->ptr) {
            this->vtable->destroy(this->ptr);
        }
    }

    explicit operator bool() const {
        return this->ptr;
    }

    void setSuccessor(Successor & successor) {
        this->vtable->setSuccessor(this->ptr, successor);
    }

    bool hasResult() const {
        return this->vtable->hasResult(this->ptr);
    }

    ValueToTuple<ResultT> & getResult() {
        return *internal::ValueFuncs<ResultT>::fromRaw(this->vtable->getResult(this->ptr));
    }

    bool hasReason() const {
        return this->vtable->hasReason(this->ptr);
    }

    ValueToTuple<ReasonT> & getReason() {
        return *internal::ValueFuncs<ReasonT>::fromRaw(this->vtable->getReason(this->ptr));
    }

private:
    internal::BrickVtable const* vtable;
    internal::RawBrick * ptr;

    template<typename OtherResultT, typename OtherReasonT>
    friend class BrickPtr;
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
