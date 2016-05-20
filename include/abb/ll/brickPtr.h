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
    void (*start)(RawBrick * brick, Successor & successor);
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
    static void start(RawBrick * brick, Successor & successor);
    static Status getStatus(RawBrick const* brick);
    static void destroy(RawBrick * brick);
    static RawValue * getResult(RawBrick * brick);
    static RawValue * getReason(RawBrick * brick);

    static const BrickVtable vtable;

private:
    typedef IsUnd<GetResult<BrickT>> IsResultUnd;
    typedef IsUnd<GetReason<BrickT>> IsReasonUnd;

    static BrickT * fromRaw(RawBrick * brick) {
        return static_cast<BrickT*>(brick);
    }

    static BrickT const* fromRaw(RawBrick const* brick) {
        return static_cast<BrickT const*>(brick);
    }

    static RawValue * getResult(RawBrick * brick, std::false_type) {
        return ValueFuncs<GetResult<BrickT>>::toRaw(&BrickFuncs::fromRaw(brick)->getResult());
    }
    static RawValue * getResult(RawBrick *, std::true_type) {
        ABB_FIASCO("Erased method called");
    }

    static RawValue * getReason(RawBrick * brick, std::false_type) {
        return ValueFuncs<GetReason<BrickT>>::toRaw(&BrickFuncs::fromRaw(brick)->getReason());
    }
    static RawValue * getReason(RawBrick *, std::true_type) {
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
void BrickFuncs<BrickT>::start(RawBrick * brick, Successor & successor) {
    BrickFuncs::fromRaw(brick)->start(successor);
}

template<typename BrickT>
Status BrickFuncs<BrickT>::getStatus(RawBrick const* brick) {
    return BrickFuncs::fromRaw(brick)->getStatus();
}

template<typename BrickT>
RawValue * BrickFuncs<BrickT>::getResult(RawBrick * brick) {
    return BrickFuncs::getResult(brick, IsResultUnd());
}

template<typename BrickT>
RawValue * BrickFuncs<BrickT>::getReason(RawBrick * brick) {
    return BrickFuncs::getReason(brick, IsReasonUnd());
}

template<typename BrickT>
const BrickVtable BrickFuncs<BrickT>::vtable = {
    &BrickFuncs::abort,
    &BrickFuncs::start,
    &BrickFuncs::getStatus,
    &BrickFuncs::destroy,
    &BrickFuncs::getResult,
    &BrickFuncs::getReason
};

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
            IsValueSubstitutable<ResultT, OtherResultT>::value &&
            IsValueSubstitutable<ReasonT, OtherReasonT>::value
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
            IsValueSubstitutable<ResultT, OtherResultT>::value &&
            IsValueSubstitutable<ReasonT, OtherReasonT>::value
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

    void start(Successor & successor) {
        this->vtable->start(this->ptr, successor);
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
    BrickPtr(internal::BrickVtable const* vtable, internal::RawBrick * ptr):
        vtable(vtable),
        ptr(ptr) {}

    internal::BrickVtable const* vtable;
    internal::RawBrick * ptr;

    template<typename FriendResultT, typename FriendReasonT>
    friend class BrickPtr;

    template<typename FriendResultT, typename FriendReasonT>
    friend BrickPtr<FriendResultT, Und> successCast(BrickPtr<FriendResultT, FriendReasonT> && brick);

    template<typename FriendResultT, typename FriendReasonT>
    friend BrickPtr<Und, FriendReasonT> errorCast(BrickPtr<FriendResultT, FriendReasonT> && brick);
};

template<typename ResultT, typename ReasonT>
inline BrickPtr<ResultT, Und> successCast(BrickPtr<ResultT, ReasonT> && brick) {
    ABB_ASSERT(brick.getStatus() & SUCCESS, "Expected success");
    internal::BrickVtable const* vtable = brick.vtable;
    internal::RawBrick * ptr = brick.ptr;
    brick.vtable = nullptr;
    brick.ptr = nullptr;
    return BrickPtr<ResultT, Und>(vtable, ptr);
}

template<typename ResultT, typename ReasonT>
inline BrickPtr<Und, ReasonT> errorCast(BrickPtr<ResultT, ReasonT> && brick) {
    ABB_ASSERT(brick.getStatus() & ERROR, "Expected error");
    internal::BrickVtable const* vtable = brick.vtable;
    internal::RawBrick * ptr = brick.ptr;
    brick.vtable = nullptr;
    brick.ptr = nullptr;
    return BrickPtr<Und, ReasonT>(vtable, ptr);
}

template<typename ArgT>
using GetBrickPtr = BrickPtr<GetResult<ArgT>, GetReason<ArgT>>;

template<typename BrickT, typename... ArgsT>
inline GetBrickPtr<BrickT> makeBrick(ArgsT &&... args) {
    return GetBrickPtr<BrickT>(new BrickT(std::forward<ArgsT>(args)...));
}

template<typename FirstPtrT, typename SecondPtrT>
using UnifyBrickPtrs = BrickPtr<
    UnifyValues<GetResult<FirstPtrT>, GetResult<SecondPtrT>>,
    UnifyValues<GetReason<FirstPtrT>, GetReason<SecondPtrT>>
>;

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_PTR_H
