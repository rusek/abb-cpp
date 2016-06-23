#ifndef ABB_LL_ANY_BRICK_H
#define ABB_LL_ANY_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/abortBrick.h>
#include <abb/utils/cord.h>
#include <abb/utils/firewalk.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class AnyBrick;

namespace internal {

template<typename ResultT, typename ReasonT>
class AnyChild : private Successor, public utils::CordNode {
private:
    typedef AnyBrick<ResultT, ReasonT> AnyBrickType;
    typedef AnyChild<ResultT, ReasonT> AnyChildType;
    typedef BrickPtr<ResultT, ReasonT> BrickPtrType;
public:
    explicit AnyChild(BrickPtrType && brick):
        parent(nullptr),
        brick(std::move(brick)) {}

    void start(AnyBrickType * parent);

    void abort();

    virtual void onUpdate();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;

    AnyBrickType * parent;
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
            this->cordRemove();
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
        for (; begin != end; ++begin) {
            this->children.insert(new ChildType(std::move(*begin)));
        }
    }

    AnyBrick():
        successor(nullptr) {}

    ~AnyBrick() {
        for (ChildType * child : utils::firewalk(this->children)) {
            delete child;
        }
    }

    void start(Successor & successor) {
        this->successor = &successor;
        for (ChildType * child : utils::firewalk(this->children)) {
            child->start(this);
        }
    }

    void abort() {
        if (!this->brick && !this->children.empty()) {
            for (ChildType * child : utils::firewalk(this->children)) {
                child->abort();
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

    utils::CordList<ChildType> children;
    BrickPtrType brick;
    Successor * successor;

    friend ChildType;
};

template<typename ResultT, typename ReasonT>
void AnyBrick<ResultT, ReasonT>::onChildFinish(BrickPtrType brick) {
    if (this->brick) {
        brick.reset();
        if (this->children.empty()) {
            this->successor->onUpdate();
        }
    } else {
        this->brick = std::move(brick);
        if (this->children.empty()) {
            this->successor->onUpdate();
        } else {
            for (ChildType * child : utils::firewalk(this->children)) {
                child->abort();
            }
        }
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_ANY_BRICK_H
