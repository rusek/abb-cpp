#ifndef ABB_LL_ANY_BRICK_H
#define ABB_LL_ANY_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/abortBrick.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class AnyBrick;

namespace internal {

template<typename ResultT, typename ReasonT>
class AnyChild : private Successor {
private:
    typedef AnyBrick<ResultT, ReasonT> AnyBrickType;
    typedef AnyChild<ResultT, ReasonT> AnyChildType;
    typedef BrickPtr<ResultT, ReasonT> BrickPtrType;
public:
    explicit AnyChild(BrickPtrType && brick):
        parent(nullptr),
        prevSibling(nullptr),
        nextSibling(nullptr),
        brick(std::move(brick)) {}

    void start(AnyBrickType * parent);

    void abort();

    virtual void onUpdate();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;

    AnyBrickType * parent;
    AnyChildType * prevSibling;
    AnyChildType * nextSibling;
    BrickPtrType brick;
};

template<typename ResultT, typename ReasonT>
void AnyChild<ResultT, ReasonT>::start(AnyBrickType * parent) {
    this->parent = parent;
    this->onUpdate();
}

template<typename ResultT, typename ReasonT>
void AnyChild<ResultT, ReasonT>::abort() {
    if (this->parent) {
        this->brick.abort();
    }
}

template<typename ResultT, typename ReasonT>
void AnyChild<ResultT, ReasonT>::onUpdate() {
    for (;;) {
        Status status = this->brick.getStatus();
        if (status == PENDING) {
            /*
            AnyBrickType * anyBrick = brickCast<AnyBrickType>(this->brick);
            if (anyBrick) {
                // FIXME not enough, we should try also AnyBrick<ResultT, Und>, AnyBrick<Und, ReasonT>
                // and AnyBrick<Und, Und>
                ABB_FIASCO("not implemented");
            } else {
                this->brick.start(*this);
            }
            */
            this->brick.start(*this);
            return;
        } else if (status & NEXT) {
            this->brick = this->brick.getNext();
        } else {
            if (this->prevSibling) {
                this->prevSibling->nextSibling = this->nextSibling;
            } else {
                this->parent->firstChild = this->nextSibling;
            }
            if (this->nextSibling) {
                this->nextSibling->prevSibling = this->prevSibling;
            } else {
                this->parent->lastChild = this->prevSibling;
            }
            AnyBrickType * parent = this->parent;
            BrickPtrType brick = std::move(this->brick);
            delete this;
            parent->onChildFinish(std::move(brick));
            return;
        }
    }
}

template<typename ResultT, typename ReasonT>
Island & AnyChild<ResultT, ReasonT>::getIsland() const {
    return this->parent->successor->getIsland();
}

template<typename ResultT, typename ReasonT>
bool AnyChild<ResultT, ReasonT>::isAborted() const {
    return this->parent->brick || this->parent->successor->isAborted();
}

} // namespace internal

template<typename ResultT, typename ReasonT>
class AnyBrick : public Brick<ResultT, ReasonT> {
private:
    typedef internal::AnyChild<ResultT, ReasonT> ChildType;
    typedef BrickPtr<ResultT, ReasonT> BrickPtrType;
public:
    template<typename IteratorT>
    AnyBrick(IteratorT begin, IteratorT end):
        successor(nullptr)
    {
        ChildType * lastChild = nullptr;
        for (; begin != end; ++begin) {
            ChildType * curChild = new ChildType(std::move(*begin));
            if (lastChild) {
                lastChild->nextSibling = curChild;
                curChild->prevSibling = lastChild;
            } else {
                this->firstChild = curChild;
            }
            lastChild = curChild;
        }
        this->lastChild = lastChild;
    }

    AnyBrick():
        firstChild(nullptr),
        lastChild(nullptr),
        successor(nullptr) {}

    ~AnyBrick() {
        for (ChildType * child = this->firstChild; child; ) {
            ChildType * nextChild = child->nextSibling;
            delete child;
            child = nextChild;
        }
    }

    void start(Successor & successor) {
        this->successor = &successor;
        for (ChildType * child = this->firstChild; child; ) {
            ChildType * nextChild = child->nextSibling;
            child->start(this);
            child = nextChild;
        }
    }

    void abort() {
        if (!this->brick && this->firstChild) {
            for (ChildType * child = this->firstChild; child; ) {
                ChildType * nextChild = child->nextSibling;
                child->abort();
                child = nextChild;
            }
        } else {
            this->brick = makeBrick<AbortBrick>();
            this->successor->onUpdate();
        }
    }

    Status getStatus() const {
        return this->brick ? NEXT : PENDING;
    }

    BrickPtrType getNext() {
        return std::move(this->brick);
    }

private:
    void onChildFinish(BrickPtrType brick);

    ChildType * firstChild;
    ChildType * lastChild;
    BrickPtrType brick;
    Successor * successor;

    friend ChildType;
};

template<typename ResultT, typename ReasonT>
void AnyBrick<ResultT, ReasonT>::onChildFinish(BrickPtrType brick) {
    if (this->brick) {
        brick.reset();
        if (!this->firstChild) {
            this->successor->onUpdate();
        }
    } else {
        this->brick = std::move(brick);
        ChildType * child = this->firstChild;
        if (child) {
            do {
                ChildType * nextChild = child->nextSibling;
                child->abort();
                child = nextChild;
            } while (child);
        } else {
            this->successor->onUpdate();
        }
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_ANY_BRICK_H
