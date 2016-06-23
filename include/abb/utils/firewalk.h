#ifndef ABB_UTILS_FIREWALK_H
#define ABB_UTILS_FIREWALK_H

#include <utility>

namespace abb {
namespace utils {

template<typename IteratorT>
class FirewalkIterator {
private:
    IteratorT wrapped;
public:
    explicit FirewalkIterator(IteratorT wrapped): wrapped(wrapped) {}

    bool operator!=(FirewalkIterator<IteratorT> const& other) const {
        return this->wrapped != other.wrapped;
    }

    FirewalkIterator<IteratorT> & operator++() {
        return *this;
    }

    auto operator*() -> decltype(*this->wrapped) {
        auto && result = *this->wrapped;
        ++this->wrapped;
        return std::move(result);
    }
};

template<typename RangeT>
class FirewalkRange {
private:
    typedef decltype(std::declval<RangeT>().begin()) IteratorType;

public:
    explicit FirewalkRange(RangeT range): range(range) {}

    FirewalkIterator<IteratorType> begin() {
        return FirewalkIterator<IteratorType>(this->range.begin());
    }

    FirewalkIterator<IteratorType> end() {
        return FirewalkIterator<IteratorType>(this->range.end());
    }

private:
    RangeT range;
};

template<typename RangeT>
inline FirewalkRange<RangeT &> firewalk(RangeT & range) {
    return FirewalkRange<RangeT &>(range);
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_FIREWALK_H
