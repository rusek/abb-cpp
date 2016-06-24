#ifndef ABB_LL_BRIDGE_H
#define ABB_LL_BRIDGE_H

#include <abb/block_fwd.h>
#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
inline base_block<Result, Reason> pack_brick_ptr(brick_ptr<Result, Reason> && brick) {
    return base_block<Result, Reason>(std::move(brick));
}

template<typename Result, typename Reason>
inline brick_ptr<Result, Reason> unpack_brick_ptr(base_block<Result, Reason> && block) {
    return block.take_brick();
}

template<typename Brick, typename... Args>
inline get_block_t<Brick> pack_brick(Args &&... args) {
    return pack_brick_ptr(make_brick<Brick>(std::forward<Args>(args)...));
}

template<typename Cont>
class unpacker {
private:
    Cont cont; // must be present before operator()

public:
    template<typename... Args>
    explicit unpacker(Args &&... args): cont(std::forward<Args>(args)...) {}

    template<typename... Args>
    auto operator()(Args &&... args) & ->
        decltype(unpack_brick_ptr(this->cont(std::forward<Args>(args)...)))
    {
        return unpack_brick_ptr(this->cont(std::forward<Args>(args)...));
    }

    template<typename... Args>
    auto operator()(Args &&... args) && ->
        decltype(unpack_brick_ptr(std::move(this->cont)(std::forward<Args>(args)...)))
    {
        return unpack_brick_ptr(std::move(this->cont)(std::forward<Args>(args)...));
    }
};

template<typename Iterator>
class unpack_iterator {
private:
    Iterator wrapped; // must be present before operator*

public:
    explicit unpack_iterator(Iterator wrapped): wrapped(wrapped) {}

    bool operator!=(unpack_iterator<Iterator> const& other) {
        return this->wrapped != other.wrapped;
    }

    unpack_iterator<Iterator> & operator++() {
        ++this->wrapped;
        return *this;
    }

    auto operator*() -> decltype(ll::unpack_brick_ptr(*this->wrapped)) {
        return ll::unpack_brick_ptr(*this->wrapped);
    }
};

template<typename Iterator>
inline unpack_iterator<Iterator> make_unpack_iterator(Iterator it) {
    return unpack_iterator<Iterator>(it);
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRIDGE_H
