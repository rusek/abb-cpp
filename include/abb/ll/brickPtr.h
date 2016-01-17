#ifndef ABB_LL_BRICK_PTR_H
#define ABB_LL_BRICK_PTR_H

#include <abb/ll/brick.h>
#include <memory>

namespace abb {
namespace ll {

typedef void RawBrick;


template<typename BrickT>
struct BrickFuncs {
    typedef typename BrickT::ResultType ResultType;
    typedef typename BrickT::ReasonType ReasonType;

    static BrickT * fromRaw(RawBrick * brick) {
        return static_cast<BrickT*>(brick);
    }

    static RawBrick * toRaw(BrickT * brick) {
        return static_cast<RawBrick*>(brick);
    }

    static BrickT const* fromRaw(RawBrick const* brick) {
        return static_cast<BrickT const*>(brick);
    }

    static void setSuccessor(RawBrick * brick, Successor & successor);
    static void destroy(RawBrick * brick);
    static bool hasResult(RawBrick const* brick);
    static ValueToTuple<ResultType> & getResult(RawBrick * brick);
    static bool hasReason(RawBrick const* brick);
    static ValueToTuple<ReasonType> & getReason(RawBrick * brick);
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
    return BrickFuncs::fromRaw(brick)->hasResult();
}

template<typename BrickT>
auto BrickFuncs<BrickT>::getResult(RawBrick * brick) -> ValueToTuple<ResultType> & {
    return BrickFuncs::fromRaw(brick)->getResult();
}

template<typename BrickT>
bool BrickFuncs<BrickT>::hasReason(RawBrick const* brick) {
    return BrickFuncs::fromRaw(brick)->hasReason();
}

template<typename BrickT>
auto BrickFuncs<BrickT>::getReason(RawBrick * brick) -> ValueToTuple<ReasonType> & {
    return BrickFuncs::fromRaw(brick)->getReason();
}


struct BrickVtableBase {
    void (*setSuccessor)(RawBrick * brick, Successor & successor);
    void (*destroy)(RawBrick * brick);
};

template<typename ResultT>
struct BrickVtableResult {
    bool (*hasResult)(RawBrick const* brick);
    ValueToTuple<ResultT> & (*getResult)(RawBrick * brick);
};

template<typename ReasonT>
struct BrickVtableReason {
    bool (*hasReason)(RawBrick const* brick);
    ValueToTuple<ReasonT> & (*getReason)(RawBrick * brick);
};

template<typename ResultT, typename ReasonT>
struct BrickVtable {
    template<typename BrickT>
    struct Def {
        static const BrickVtable vtable;
    };

    BrickVtableBase base;
    BrickVtableResult<ResultT> result;
    BrickVtableReason<ReasonT> reason;
};

template<typename ResultT, typename ReasonT>
template<typename BrickT>
const BrickVtable<ResultT, ReasonT> BrickVtable<ResultT, ReasonT>::Def<BrickT>::vtable = {{
    &BrickFuncs<BrickT>::setSuccessor,
    &BrickFuncs<BrickT>::destroy
}, {
    &BrickFuncs<BrickT>::hasResult,
    &BrickFuncs<BrickT>::getResult
}, {
    &BrickFuncs<BrickT>::hasReason,
    &BrickFuncs<BrickT>::getReason
}};

template<typename ResultT>
struct BrickVtable<ResultT, Und> {
    template<typename BrickT>
    struct Def {
        static const BrickVtable vtable;
    };

    BrickVtableBase base;
    BrickVtableResult<ResultT> result;
};

template<typename ResultT>
template<typename BrickT>
const BrickVtable<ResultT, Und> BrickVtable<ResultT, Und>::Def<BrickT>::vtable = {{
    &BrickFuncs<BrickT>::setSuccessor,
    &BrickFuncs<BrickT>::destroy
}, {
    &BrickFuncs<BrickT>::hasResult,
    &BrickFuncs<BrickT>::getResult
}};

template<typename ReasonT>
struct BrickVtable<Und, ReasonT> {
    template<typename BrickT>
    struct Def {
        static const BrickVtable vtable;
    };

    BrickVtableBase base;
    BrickVtableReason<ReasonT> reason;
};

template<typename ReasonT>
template<typename BrickT>
const BrickVtable<Und, ReasonT> BrickVtable<Und, ReasonT>::Def<BrickT>::vtable = {{
    &BrickFuncs<BrickT>::setSuccessor,
    &BrickFuncs<BrickT>::destroy
}, {
    &BrickFuncs<BrickT>::hasReason,
    &BrickFuncs<BrickT>::getReason
}};


template<typename ResultT, typename ReasonT>
class BrickPtr {
public:
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;

    BrickPtr(): vtable(nullptr), ptr(nullptr) {}

    template<typename BrickT>
    explicit BrickPtr(BrickT * brick):
        vtable(&BrickVtable<ResultT, ReasonT>::template Def<BrickT>::vtable),
        ptr(BrickFuncs<BrickT>::toRaw(brick)) {}

    BrickPtr(BrickPtr const&) = delete;
    BrickPtr(BrickPtr && other):
        vtable(other.vtable),
        ptr(other.ptr)
    {
        other.vtable = nullptr;
        other.ptr = nullptr;
    }

    BrickPtr & operator=(BrickPtr const&) = delete;
    BrickPtr & operator=(BrickPtr && other) {
        if (this->ptr) {
            this->vtable->base.destroy(this->ptr);
        }
        this->vtable = other.vtable;
        this->ptr = other.ptr;
        other.vtable = nullptr;
        other.ptr = nullptr;
        return *this;
    }

    ~BrickPtr() {
        if (this->ptr) {
            this->vtable->base.destroy(this->ptr);
        }
    }

    explicit operator bool() const {
        return this->ptr;
    }

    void setSuccessor(Successor & successor) {
        this->vtable->base.setSuccessor(this->ptr, successor);
    }

    bool hasResult() const {
        return this->vtable->result.hasResult(this->ptr);
    }

    ValueToTuple<ResultT> & getResult() {
        return this->vtable->result.getResult(this->ptr);
    }

    bool hasReason() const {
        return this->vtable->reason.hasReason(this->ptr);
    }

    ValueToTuple<ReasonT> & getReason() {
        return this->vtable->reason.getReason(this->ptr);
    }

private:
    BrickVtable<ResultT, ReasonT> const* vtable;
    RawBrick * ptr;
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
