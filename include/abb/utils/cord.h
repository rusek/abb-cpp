#ifndef ABB_UTILS_CORD_H
#define ABB_UTILS_CORD_H

#include <abb/utils/noncopyable.h>
#include <type_traits>

namespace abb {
namespace utils {

struct cord_node : noncopyable {
    cord_node():
        cord_prev(this),
        cord_next(this) {}

    void cord_insert(cord_node * elem) {
        cord_node * this_prev = this->cord_prev;
        cord_node * elem_prev = elem->cord_prev;

        elem->cord_prev = this_prev;
        this->cord_prev = elem_prev;
        this_prev->cord_next = elem;
        elem_prev->cord_next = this;
    }

    void cord_detach() {
        cord_node * this_prev = this->cord_prev;
        cord_node * this_next = this->cord_next;

        this_prev->cord_next = this_next;
        this_next->cord_prev = this_prev;
        this->cord_next = this->cord_prev = this;
    }

    cord_node * cord_prev;
    cord_node * cord_next;
};

template<typename Obj>
class cord_iterator {
private:
    typedef typename std::conditional<
        std::is_const<Obj>::value,
        cord_node const*,
        cord_node *
    >::type cord_node_ptr;

public:
    explicit cord_iterator(cord_node_ptr elem): elem(elem) {}

    bool operator!=(cord_iterator<Obj> const& other) const {
        return this->elem != other.elem;
    }

    cord_iterator<Obj> & operator++() {
        this->elem = this->elem->cord_next;
        return *this;
    }

    Obj * operator*() const {
        return static_cast<Obj *>(this->elem);
    }

private:
    cord_node_ptr elem;
};

template<typename Obj>
class cord_list : public cord_node {
public:
    typedef cord_iterator<Obj> iterator;
    typedef cord_iterator<Obj const> const_iterator;

    void insert(Obj * obj) {
        this->cord_insert(obj);
    }

    bool empty() const {
        return this == this->cord_next;
    }

    iterator begin() {
        return iterator(this->cord_next);
    }

    const_iterator begin() const {
        return const_iterator(this->cord_next);
    }

    iterator end() {
        return iterator(this);
    }

    const_iterator end() const {
        return const_iterator(this);
    }
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_CORD_H
