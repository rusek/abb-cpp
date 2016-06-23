#ifndef ABB_UTILS_CORD_H
#define ABB_UTILS_CORD_H

#include <abb/utils/noncopyable.h>
#include <type_traits>

namespace abb {
namespace utils {

struct CordNode : Noncopyable {
    CordNode():
        cordPrev(this),
        cordNext(this) {}

    void cordInsert(CordNode * elem) {
        CordNode * thisPrev = this->cordPrev;
        CordNode * elemPrev = elem->cordPrev;

        elem->cordPrev = thisPrev;
        this->cordPrev = elemPrev;
        thisPrev->cordNext = elem;
        elemPrev->cordNext = this;
    }

    void cordRemove() {
        CordNode * thisPrev = this->cordPrev;
        CordNode * thisNext = this->cordNext;

        thisPrev->cordNext = thisNext;
        thisNext->cordPrev = thisPrev;
        this->cordNext = this->cordPrev = this;
    }

    CordNode * cordPrev;
    CordNode * cordNext;
};

template<typename ObjT>
class CordIterator {
private:
    typedef typename std::conditional<std::is_const<ObjT>::value, CordNode const*, CordNode *>::type CordNodePtr;

public:
    explicit CordIterator(CordNodePtr elem): elem(elem) {}

    bool operator!=(CordIterator<ObjT> const& other) const {
        return this->elem != other.elem;
    }

    CordIterator<ObjT> & operator++() {
        this->elem = this->elem->cordNext;
        return *this;
    }

    ObjT * operator*() const {
        return static_cast<ObjT *>(this->elem);
    }

private:
    CordNodePtr elem;
};

template<typename ObjT>
class CordList : public CordNode {
public:
    typedef CordIterator<ObjT> iterator;
    typedef CordIterator<ObjT const> const_iterator;

    void insert(ObjT * obj) {
        this->cordInsert(obj);
    }

    bool empty() const {
        return this == this->cordNext;
    }

    iterator begin() {
        return iterator(this->cordNext);
    }

    const_iterator begin() const {
        return const_iterator(this->cordNext);
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
