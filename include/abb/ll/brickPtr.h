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
    void (*abort)(RawBrick * brick);
    void (*run)(RawBrick * brick, Successor & successor);
    Status (*getStatus)(RawBrick const* brick);
    void (*destroy)(RawBrick * brick);
    // Success-related methods
    RawValue * (*getResult)(RawBrick * brick);
    // Error-related methods
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

    static void abort(RawBrick * brick);
    static void run(RawBrick * brick, Successor & successor);
    static Status getStatus(RawBrick const* brick);
    static void destroy(RawBrick * brick);
    static RawValue * getResult(RawBrick * brick);
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

    static RawValue * getResult(RawBrick * brick, std::true_type) {
        return ValueFuncs<typename BrickT::ResultType>::toRaw(&BrickFuncs::fromRaw(brick)->getResult());
    }
    static RawValue * getResult(RawBrick *, std::false_type) {
        ABB_FIASCO("Erased method called");
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
void BrickFuncs<BrickT>::abort(RawBrick * brick) {
    BrickFuncs::fromRaw(brick)->abort();
}

template<typename BrickT>
void BrickFuncs<BrickT>::run(RawBrick * brick, Successor & successor) {
    BrickFuncs::fromRaw(brick)->run(successor);
}

template<typename BrickT>
Status BrickFuncs<BrickT>::getStatus(RawBrick const* brick) {
    return BrickFuncs::fromRaw(brick)->getStatus();
}

template<typename BrickT>
RawValue * BrickFuncs<BrickT>::getResult(RawBrick * brick) {
    return BrickFuncs::getResult(brick, ResultDefined());
}

template<typename BrickT>
RawValue * BrickFuncs<BrickT>::getReason(RawBrick * brick) {
    return BrickFuncs::getReason(brick, ReasonDefined());
}

template<typename BrickT>
const BrickVtable BrickFuncs<BrickT>::vtable = {
    &BrickFuncs::abort,
    &BrickFuncs::run,
    &BrickFuncs::getStatus,
    &BrickFuncs::destroy,
    &BrickFuncs::getResult,
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

    template<
        typename OtherResultT,
        typename OtherReasonT,
        typename std::enable_if<
            internal::IsValueSubstitutable<ResultT, OtherResultT>::value &&
            internal::IsValueSubstitutable<ReasonT, OtherReasonT>::value
        >::type* = nullptr
    > BrickPtr & operator=(BrickPtr<OtherResultT, OtherReasonT> && other) {
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

    void abort() {
        this->vtable->abort(this->ptr);
    }

    void run(Successor & successor) {
        this->vtable->run(this->ptr, successor);
    }

    Status getStatus() const {
        return this->vtable->getStatus(this->ptr);
    }

    ValueToTuple<ResultT> & getResult() {
        return *internal::ValueFuncs<ResultT>::fromRaw(this->vtable->getResult(this->ptr));
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
