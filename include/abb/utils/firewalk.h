#ifndef ABB_UTILS_FIREWALK_H
#define ABB_UTILS_FIREWALK_H

#include <utility>

namespace abb {
namespace utils {

template<typename Iterator>
class firewalk_iterator {
private:
    Iterator wrapped;
public:
    explicit firewalk_iterator(Iterator wrapped): wrapped(wrapped) {}

    bool operator!=(firewalk_iterator<Iterator> const& other) const {
        return this->wrapped != other.wrapped;
    }

    firewalk_iterator<Iterator> & operator++() {
        return *this;
    }

    auto operator*() -> decltype(*this->wrapped) {
        auto && result = *this->wrapped;
        ++this->wrapped;
        return std::move(result);
    }
};

template<typename Range>
class firewalk_range {
private:
    typedef decltype(std::declval<Range>().begin()) iterator;

public:
    explicit firewalk_range(Range range): range(range) {}

    firewalk_iterator<iterator> begin() {
        return firewalk_iterator<iterator>(this->range.begin());
    }

    firewalk_iterator<iterator> end() {
        return firewalk_iterator<iterator>(this->range.end());
    }

private:
    Range range;
};

template<typename Range>
inline firewalk_range<Range &> firewalk(Range & range) {
    return firewalk_range<Range &>(range);
}

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_FIREWALK_H
