#ifndef ABB_ANY_H
#define ABB_ANY_H

#include <abb/ll/anyBrick.h>
#include <abb/ll/bridge.h>

namespace abb {

namespace internal {

template<typename IteratorT>
using AnyOfReturn = typename std::decay<decltype(*std::declval<IteratorT>())>::type;

using std::begin;
using std::end;

template<typename RangeT>
struct AnyOfRangeTraits {};

template<typename RangeT>
struct AnyOfRangeTraits<RangeT &> {
    typedef decltype(begin(std::declval<RangeT&>())) IteratorType;
    typedef AnyOfReturn<IteratorType> BlockType;

    static IteratorType doBegin(RangeT & range) {
        return begin(range);
    }

    static IteratorType doEnd(RangeT & range) {
        return end(range);
    }
};

template<typename RangeT>
struct AnyOfRangeTraits<RangeT &&> {
    typedef std::move_iterator<decltype(begin(std::declval<RangeT&>()))> IteratorType;
    typedef AnyOfReturn<IteratorType> BlockType;

    static IteratorType doBegin(RangeT && range) {
        return std::make_move_iterator(begin(range));
    }

    static IteratorType doEnd(RangeT && range) {
        return std::make_move_iterator(end(range));
    }
};

} // namespace internal

template<typename... ResultsT, typename... ReasonsT>
BaseBlock<CommonValue<ResultsT...>, CommonValue<ReasonsT...>> any(BaseBlock<ResultsT, ReasonsT> &&... blocks) {
    typedef BaseBlock<CommonValue<ResultsT...>, CommonValue<ReasonsT...>> BlockType;
    typedef ll::AnyBrick<GetResult<BlockType>, GetReason<BlockType>> AnyBrickType;
    ll::GetBrickPtr<BlockType> bricks[] = {ll::unpackBrickPtr(std::move(blocks))...};

    return ll::packBrick<AnyBrickType>(std::begin(bricks), std::end(bricks));
}

template<typename IteratorT>
internal::AnyOfReturn<IteratorT> anyOf(IteratorT begin, IteratorT end) {
    typedef internal::AnyOfReturn<IteratorT> BlockType;
    typedef ll::AnyBrick<GetResult<BlockType>, GetReason<BlockType>> AnyBrickType;

    return ll::packBrick<AnyBrickType>(ll::makeUnpackIterator(begin), ll::makeUnpackIterator(end));
}

template<typename RangeT>
typename internal::AnyOfRangeTraits<RangeT &&>::BlockType anyOf(RangeT && range) {
    typedef internal::AnyOfRangeTraits<RangeT &&> Traits;
    return anyOf(Traits::doBegin(std::forward<RangeT>(range)), Traits::doEnd(std::forward<RangeT>(range)));
}

UndBlock hold();

} // namespace abb

#endif // ABB_ANY_H
