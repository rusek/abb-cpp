#ifndef ABB_LL_BRIDGE_H
#define ABB_LL_BRIDGE_H

#include <abb/blockFwd.h>
#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
inline BaseBlock<ResultT, ReasonT> packBrickPtr(BrickPtr<ResultT, ReasonT> && brick) {
    return BaseBlock<ResultT, ReasonT>(std::move(brick));
}

template<typename ResultT, typename ReasonT>
inline BrickPtr<ResultT, ReasonT> unpackBrickPtr(BaseBlock<ResultT, ReasonT> && block) {
    return block.takeBrick();
}

template<typename BrickT, typename... ArgsT>
inline GetBlock<BrickT> packBrick(ArgsT &&... args) {
    return packBrickPtr(makeBrick<BrickT>(std::forward<ArgsT>(args)...));
}

template<typename ContT>
class Unpacker {
private:
    ContT cont; // must be present before operator()

public:
    template<typename... ArgsT>
    explicit Unpacker(ArgsT &&... args): cont(std::forward<ArgsT>(args)...) {}

    template<typename... ArgsT>
    auto operator()(ArgsT &&... args) && ->
        decltype(unpackBrickPtr(std::move(this->cont)(std::forward<ArgsT>(args)...)))
    {
        return unpackBrickPtr(std::move(this->cont)(std::forward<ArgsT>(args)...));
    }
};

template<typename IteratorT>
class UnpackIterator {
private:
    IteratorT wrapped; // must be present before operator*

public:
    explicit UnpackIterator(IteratorT wrapped): wrapped(wrapped) {}

    bool operator!=(UnpackIterator<IteratorT> const& other) {
        return this->wrapped != other.wrapped;
    }

    UnpackIterator<IteratorT> & operator++() {
        ++this->wrapped;
        return *this;
    }

    auto operator*() -> decltype(ll::unpackBrickPtr(*this->wrapped)) {
        return ll::unpackBrickPtr(*this->wrapped);
    }
};

template<typename IteratorT>
inline UnpackIterator<IteratorT> makeUnpackIterator(IteratorT it) {
    return UnpackIterator<IteratorT>(it);
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_BRIDGE_H
