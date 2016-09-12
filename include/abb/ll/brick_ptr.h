#ifndef ABB_LL_BRICK_PTR_H
#define ABB_LL_BRICK_PTR_H

#include <abb/ll/brick.h>
#include <memory>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class brick_ptr;

template<typename Arg>
using get_brick_ptr_t = brick_ptr<get_result_t<Arg>, get_reason_t<Arg>>;

namespace internal {

typedef void raw_value;

struct brick_vtable {
    // Base methods
    void (*abort)(raw_brick * brick);
    void (*start)(raw_brick * brick, island & target, bool aborted, successor & succ);
    status (*get_status)(raw_brick const* brick);
    raw_brick * (*get_next)(raw_brick * brick);
    void (*destroy)(raw_brick * brick);
    // Success-related methods
    raw_value * (*get_result)(raw_brick * brick);
    // Error-related methods
    raw_value * (*get_reason)(raw_brick * brick);
};


template<typename Value>
struct value_funcs {
    static raw_value * to_raw(store<Value> * value) {
        return static_cast<raw_value*>(value);
    }

    static store<Value> * from_raw(raw_value * value) {
        return static_cast<store<Value>*>(value);
    }
};

template<typename Brick>
struct brick_funcs {
    static raw_brick * to_raw(Brick * brick) {
        return static_cast<raw_brick*>(brick);
    }

    static Brick * from_raw(raw_brick * brick) {
        return static_cast<Brick*>(brick);
    }

    static Brick const* from_raw(raw_brick const* brick) {
        return static_cast<Brick const*>(brick);
    }

    static void abort(raw_brick * brick);
    static void start(raw_brick * brick, island & island, bool aborted, successor & succ);
    static status get_status(raw_brick const* brick);
    static raw_brick * get_next(raw_brick * brick);
    static void destroy(raw_brick * brick);
    static raw_value * get_result(raw_brick * brick);
    static raw_value * get_reason(raw_brick * brick);

    static const brick_vtable vtable;

private:
    typedef is_und<get_result_t<Brick>> is_result_und;
    typedef is_und<get_reason_t<Brick>> is_reason_und;

    static raw_value * get_result(raw_brick * brick, std::false_type) {
        return value_funcs<get_result_t<Brick>>::to_raw(&brick_funcs::from_raw(brick)->get_result());
    }
    static raw_value * get_result(raw_brick *, std::true_type) {
        ABB_FIASCO("Erased method called");
    }

    static raw_value * get_reason(raw_brick * brick, std::false_type) {
        return value_funcs<get_reason_t<Brick>>::to_raw(&brick_funcs::from_raw(brick)->get_reason());
    }
    static raw_value * get_reason(raw_brick *, std::true_type) {
        ABB_FIASCO("Erased method called");
    }
};

template<typename Brick>
void brick_funcs<Brick>::destroy(raw_brick * brick) {
    delete brick_funcs::from_raw(brick);
}

template<typename Brick>
void brick_funcs<Brick>::abort(raw_brick * brick) {
    brick_funcs::from_raw(brick)->abort();
}

template<typename Brick>
void brick_funcs<Brick>::start(raw_brick * brick, island & target, bool aborted, successor & succ) {
    brick_funcs::from_raw(brick)->start(target, aborted, succ);
}

template<typename Brick>
status brick_funcs<Brick>::get_status(raw_brick const* brick) {
    return brick_funcs::from_raw(brick)->get_status();
}

template<typename Brick>
raw_brick * brick_funcs<Brick>::get_next(raw_brick * brick) {
    get_brick_ptr_t<Brick> next_brick = brick_funcs::from_raw(brick)->get_next();
    raw_brick * ptr = next_brick.ptr;
    next_brick.ptr = nullptr;
    return ptr;
}

template<typename Brick>
raw_value * brick_funcs<Brick>::get_result(raw_brick * brick) {
    return brick_funcs::get_result(brick, is_result_und());
}

template<typename Brick>
raw_value * brick_funcs<Brick>::get_reason(raw_brick * brick) {
    return brick_funcs::get_reason(brick, is_reason_und());
}

template<typename Brick>
const brick_vtable brick_funcs<Brick>::vtable = {
    &brick_funcs::abort,
    &brick_funcs::start,
    &brick_funcs::get_status,
    &brick_funcs::get_next,
    &brick_funcs::destroy,
    &brick_funcs::get_result,
    &brick_funcs::get_reason
};

} // namespace internal

template<typename Result, typename Reason>
class brick_ptr {
public:
    typedef Result result;
    typedef Reason reason;

    brick_ptr(): ptr(nullptr) {}

    template<typename Brick>
    explicit brick_ptr(Brick * brick):
        ptr(internal::brick_funcs<Brick>::to_raw(brick))
    {
        this->ptr->vtable = &internal::brick_funcs<Brick>::vtable;
    }

    brick_ptr(brick_ptr const&) = delete;

    template<
        typename OtherResult,
        typename OtherReason,
        typename std::enable_if<
            is_value_substitutable<Result, OtherResult>::value &&
            is_value_substitutable<Reason, OtherReason>::value
        >::type* = nullptr
    > brick_ptr(brick_ptr<OtherResult, OtherReason> && other):
        ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    brick_ptr & operator=(brick_ptr const&) = delete;

    template<
        typename OtherResult,
        typename OtherReason,
        typename std::enable_if<
            is_value_substitutable<Result, OtherResult>::value &&
            is_value_substitutable<Reason, OtherReason>::value
        >::type* = nullptr
    > brick_ptr & operator=(brick_ptr<OtherResult, OtherReason> && other) {
        if (this->ptr) {
            this->ptr->vtable->destroy(this->ptr);
        }
        this->ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    ~brick_ptr() {
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

    void start(island & target, bool aborted, successor & succ) {
        this->ptr->vtable->start(this->ptr, target, aborted, succ);
    }

    status get_status() const {
        return this->ptr->vtable->get_status(this->ptr);
    }

    brick_ptr<Result, Reason> get_next() {
        return brick_ptr<Result, Reason>(this->ptr->vtable->get_next(this->ptr));
    }

    store<Result> & get_result() {
        return *internal::value_funcs<Result>::from_raw(this->ptr->vtable->get_result(this->ptr));
    }

    store<Reason> & get_reason() {
        return *internal::value_funcs<Reason>::from_raw(this->ptr->vtable->get_reason(this->ptr));
    }

    status update(island & target, bool aborted, successor & succ);

private:
    explicit brick_ptr(internal::raw_brick * ptr):
        ptr(ptr) {}

    internal::raw_brick * ptr;

    template<typename FriendResult, typename FriendReason>
    friend class brick_ptr;

    template<typename FriendBrick>
    friend class internal::brick_funcs;

    template<typename FriendResult, typename FriendReason>
    friend brick_ptr<und_t, und_t> abort_cast(brick_ptr<FriendResult, FriendReason> && brick);

    template<typename FriendResult, typename FriendReason>
    friend brick_ptr<FriendResult, und_t> success_cast(brick_ptr<FriendResult, FriendReason> && brick);

    template<typename FriendResult, typename FriendReason>
    friend brick_ptr<und_t, FriendReason> error_cast(brick_ptr<FriendResult, FriendReason> && brick);

    template<typename FriendBrick>
    friend FriendBrick * brick_cast(get_brick_ptr_t<FriendBrick> & brick);
};

template<typename Result, typename Reason>
status brick_ptr<Result, Reason>::update(island & target, bool aborted, successor & succ) {
    for (;;) {
        status cur_status = this->get_status();
        switch (cur_status) {
        case status::startable:
            this->start(target, aborted, succ);
            break;
        case status::next:
            (*this) = this->get_next();
            break;
        default:
            return cur_status;
        }
    }
}


template<typename Result, typename Reason>
inline brick_ptr<und_t, und_t> abort_cast(brick_ptr<Result, Reason> && brick) {
    ABB_ASSERT(brick.get_status() == status::abort, "Expected abort");
    internal::raw_brick * ptr = brick.ptr;
    brick.ptr = nullptr;
    return brick_ptr<und_t, und_t>(ptr);
}

template<typename Result, typename Reason>
inline brick_ptr<Result, und_t> success_cast(brick_ptr<Result, Reason> && brick) {
    ABB_ASSERT(brick.get_status() == status::success, "Expected success");
    internal::raw_brick * ptr = brick.ptr;
    brick.ptr = nullptr;
    return brick_ptr<Result, und_t>(ptr);
}

template<typename Result, typename Reason>
inline brick_ptr<und_t, Reason> error_cast(brick_ptr<Result, Reason> && brick) {
    ABB_ASSERT(brick.get_status() == status::error, "Expected error");
    internal::raw_brick * ptr = brick.ptr;
    brick.ptr = nullptr;
    return brick_ptr<und_t, Reason>(ptr);
}

template<typename Brick>
Brick * brick_cast(get_brick_ptr_t<Brick> & brick) {
    if (brick.ptr->vtable == &internal::brick_funcs<Brick>::vtable) {
        return internal::brick_funcs<Brick>::from_raw(brick.ptr);
    } else {
        return nullptr;
    }
}

template<typename Brick, typename... Args>
inline get_brick_ptr_t<Brick> make_brick(Args &&... args) {
    return get_brick_ptr_t<Brick>(new Brick(std::forward<Args>(args)...));
}

template<typename FirstPtr, typename SecondPtr>
using common_brick_ptr_t = brick_ptr<
    common_value_t<get_result_t<FirstPtr>, get_result_t<SecondPtr>>,
    common_value_t<get_reason_t<FirstPtr>, get_reason_t<SecondPtr>>
>;

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRICK_PTR_H
