#ifndef ABB_LL_BRICK_PTR_H
#define ABB_LL_BRICK_PTR_H

#include <abb/ll/brick.h>
#include <memory>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class BrickPtr;

template<typename ArgT>
using GetBrickPtr = BrickPtr<GetResult<ArgT>, GetReason<ArgT>>;

namespace internal {

typedef void RawValue;

struct BrickVtable {
    // Base methods
    void (*abort)(RawBrick * brick);
    void (*start)(RawBrick * brick, Successor & successor);
    Status (*getStatus)(RawBrick const* brick);
    RawBrick * (*getNext)(RawBrick * brick);
    void (*destroy)(RawBrick * brick);
    // Success-related methods
    RawValue * (*getResult)(RawBrick * brick);
    // Error-related methods
    RawValue * (*getReason)(RawBrick * brick);
};


template<typename ValueT>
struct ValueFuncs {
    static RawValue * toRaw(Store<ValueT> * value) {
        return static_cast<RawValue*>(value);
    }

    static Store<ValueT> * fromRaw(RawValue * value) {
        return static_cast<Store<ValueT>*>(value);
    }
};

template<typename BrickT>
struct BrickFuncs {
    static RawBrick * toRaw(BrickT * brick) {
        return static_cast<RawBrick*>(brick);
    }

    static BrickT * fromRaw(RawBrick * brick) {
        return static_cast<BrickT*>(brick);
    }

    static BrickT const* fromRaw(RawBrick const* brick) {
        return static_cast<BrickT const*>(brick);
    }

    static void abort(RawBrick * brick);
    static void start(RawBrick * brick, Successor & successor);
    static Status getStatus(RawBrick const* brick);
    static RawBrick * getNext(RawBrick * brick);
    static void destroy(RawBrick * brick);
    static RawValue * getResult(RawBrick * brick);
    static RawValue * getReason(RawBrick * brick);

    static const BrickVtable vtable;

private:
    typedef IsUnd<GetResult<BrickT>> IsResultUnd;
    typedef IsUnd<GetReason<BrickT>> IsReasonUnd;

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
RawBrick * BrickFuncs<BrickT>::getNext(RawBrick * brick) {
    GetBrickPtr<BrickT> nextBrick = BrickFuncs::fromRaw(brick)->getNext();
    RawBrick * ptr = nextBrick.ptr;
    nextBrick.ptr = nullptr;
    return ptr;
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
    &BrickFuncs::getNext,
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

    BrickPtr(): ptr(nullptr) {}

    template<typename BrickT>
    explicit BrickPtr(BrickT * brick):
        ptr(internal::BrickFuncs<BrickT>::toRaw(brick))
    {
        this->ptr->vtable = &internal::BrickFuncs<BrickT>::vtable;
    }

    BrickPtr(BrickPtr const&) = delete;

    template<
        typename OtherResultT,
        typename OtherReasonT,
        typename std::enable_if<
            IsValueSubstitutable<ResultT, OtherResultT>::value &&
            IsValueSubstitutable<ReasonT, OtherReasonT>::value
        >::type* = nullptr
    > BrickPtr(BrickPtr<OtherResultT, OtherReasonT> && other):
        ptr(other.ptr)
    {
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
            this->ptr->vtable->destroy(this->ptr);
        }
        this->ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    ~BrickPtr() {
        if (this->ptr) {
            this->ptr->vtable->destroy(this->ptr);
        }
    }

    void reset() {
        if (this->ptr) {
            this->ptr->vtable->destroy(this->ptr);
            this->ptr = nullptr;
        }
    }

    explicit operator bool() const {
        return this->ptr;
    }

    void abort() {
        this->ptr->vtable->abort(this->ptr);
    }

    void start(Successor & successor) {
        this->ptr->vtable->start(this->ptr, successor);
    }

    Status getStatus() const {
        return this->ptr->vtable->getStatus(this->ptr);
    }

    BrickPtr<ResultT, ReasonT> getNext() {
        return BrickPtr<ResultT, ReasonT>(this->ptr->vtable->getNext(this->ptr));
    }

    Store<ResultT> & getResult() {
        return *internal::ValueFuncs<ResultT>::fromRaw(this->ptr->vtable->getResult(this->ptr));
    }

    Store<ReasonT> & getReason() {
        return *internal::ValueFuncs<ReasonT>::fromRaw(this->ptr->vtable->getReason(this->ptr));
    }

private:
    explicit BrickPtr(internal::RawBrick * ptr):
        ptr(ptr) {}

    internal::RawBrick * ptr;

    template<typename FriendResultT, typename FriendReasonT>
    friend class BrickPtr;

    template<typename FriendBrickT>
    friend class internal::BrickFuncs;

    template<typename FriendResultT, typename FriendReasonT>
    friend BrickPtr<FriendResultT, Und> successCast(BrickPtr<FriendResultT, FriendReasonT> && brick);

    template<typename FriendResultT, typename FriendReasonT>
    friend BrickPtr<Und, FriendReasonT> errorCast(BrickPtr<FriendResultT, FriendReasonT> && brick);

    template<typename FriendBrickT>
    friend FriendBrickT * brickCast(GetBrickPtr<FriendBrickT> & brick);
};

template<typename ResultT, typename ReasonT>
inline BrickPtr<ResultT, Und> successCast(BrickPtr<ResultT, ReasonT> && brick) {
    ABB_ASSERT(brick.getStatus() & SUCCESS, "Expected success");
    internal::RawBrick * ptr = brick.ptr;
    brick.ptr = nullptr;
    return BrickPtr<ResultT, Und>(ptr);
}

template<typename ResultT, typename ReasonT>
inline BrickPtr<Und, ReasonT> errorCast(BrickPtr<ResultT, ReasonT> && brick) {
    ABB_ASSERT(brick.getStatus() & ERROR, "Expected error");
    internal::RawBrick * ptr = brick.ptr;
    brick.ptr = nullptr;
    return BrickPtr<Und, ReasonT>(ptr);
}

template<typename BrickT>
BrickT * brickCast(GetBrickPtr<BrickT> & brick) {
    if (brick.ptr->vtable == &internal::BrickFuncs<BrickT>::vtable) {
        return internal::BrickFuncs<BrickT>::fromRaw(brick.ptr);
    } else {
        return nullptr;
    }
}

template<typename BrickT, typename... ArgsT>
inline GetBrickPtr<BrickT> makeBrick(ArgsT &&... args) {
    return GetBrickPtr<BrickT>(new BrickT(std::forward<ArgsT>(args)...));
}

template<typename FirstPtrT, typename SecondPtrT>
using CommonBrickPtr = BrickPtr<
    CommonValue<GetResult<FirstPtrT>, GetResult<SecondPtrT>>,
    CommonValue<GetReason<FirstPtrT>, GetReason<SecondPtrT>>
>;

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_PTR_H
