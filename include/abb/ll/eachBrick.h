#ifndef ABB_LL_EACH_BRICK_H
#define ABB_LL_EACH_BRICK_H

#include <abb/inplace.h>
#include <abb/ll/brick.h>
#include <abb/ll/brickPtr.h>
#include <abb/ll/store.h>
#include <abb/ll/successBrick.h>
#include <abb/utils/noncopyable.h>
#include <abb/utils/cord.h>
#include <abb/utils/firewalk.h>
#include <abb/utils/range.h>

namespace abb {
namespace ll {

template<typename RangeT>
class RangeGenerator : utils::Noncopyable {
private:
    typedef decltype(utils::begin(std::declval<RangeT&>())) IteratorType;
    typedef decltype(*std::declval<IteratorType>()) ElementType;

public:
    explicit RangeGenerator(RangeT && range):
        range(std::forward<RangeT>(range)),
        it(utils::begin(this->range)),
        end(utils::end(this->range)) {}

    explicit RangeGenerator(RangeT const& range):
        range(range),
        it(utils::begin(this->range)),
        end(utils::end(this->range)) {}

    explicit operator bool() const {
        return this->it != this->end;
    }

    ElementType operator()() {
        ElementType elem = *this->it;
        ++this->it;
        return std::forward<ElementType>(elem);
    }

private:
    RangeT range;
    IteratorType it;
    IteratorType end;
};

template<typename IteratorT>
class IteratorGenerator {
private:
    typedef decltype(*std::declval<IteratorT>()) ElementType;

public:
    IteratorGenerator(IteratorT begin, IteratorT end):
        it(begin),
        end(end) {}

    explicit operator bool() const {
        return this->it != this->end;
    }

    ElementType operator()() {
        ElementType elem = *this->it;
        ++this->it;
        return std::forward<ElementType>(elem);
    }

private:
    IteratorT it;
    IteratorT end;
};

template<typename GeneratorT, typename FuncT>
class MapGenerator {
private:
    GeneratorT wrapped;
    FuncT func;

public:
    template<typename... GeneratorItemsT, typename... FuncItemsT>
    MapGenerator(Inplace<GeneratorItemsT...> wrappedArgs, Inplace<FuncItemsT...> funcArgs):
        wrapped(GeneratorItemsT::get(wrappedArgs)...),
        func(FuncItemsT::get(funcArgs)...) {}

    explicit operator bool() const {
        return bool(this->wrapped);
    }

    auto operator()() -> decltype(this->func(this->wrapped())) {
        return this->func(this->wrapped());
    }
};

namespace internal {

template<typename EachBrickT>
class EachChild : private Successor, public utils::CordNode {
private:
    typedef GetBrickPtr<EachBrickT> BrickPtrType;

public:
    explicit EachChild(BrickPtrType brick): brick(std::move(brick)) {}

    void start(EachBrickT * parent);

private:
    virtual void onUpdate();
    virtual Island & getIsland() const;
    virtual bool isAborted() const;

    BrickPtrType brick;
    EachBrickT * parent;
};

template<typename EachBrickT>
void EachChild<EachBrickT>::start(EachBrickT * parent) {
    this->parent = parent;
    this->onUpdate();
}

template<typename EachBrickT>
void EachChild<EachBrickT>::onUpdate() {
    for (;;) {
        Status status = this->brick.getStatus();
        if (status == PENDING) {
            this->brick.start(*this);
            return;
        } else if (status & NEXT) {
            this->brick = this->brick.getNext();
        } else if (status & SUCCESS) {
            if (this->parent->generator) {
                this->brick = this->parent->generator();
            } else {
                BrickPtrType brick = std::move(this->brick);
                EachBrickT * parent = this->parent;
                this->cordRemove();
                delete this;
                parent->onChildSuccess(std::move(brick));
                return;
            }
        } else {
            BrickPtrType brick = std::move(this->brick);
            EachBrickT * parent = this->parent;
            this->cordRemove();
            delete this;
            parent->onChildError(std::move(brick));
            return;
        }
    }
}

template<typename EachBrickT>
Island & EachChild<EachBrickT>::getIsland() const {
    return this->parent->successor->getIsland();
}

template<typename EachBrickT>
bool EachChild<EachBrickT>::isAborted() const {
    return this->parent->brick || this->parent->successor->isAborted();
}

} // namespace internal

template<typename GeneratorT>
class EachBrick : public Brick<void(), typename decltype(std::declval<GeneratorT>()())::ReasonType> {
public:
    typedef void ResultType();
    typedef typename decltype(std::declval<GeneratorT>()())::ReasonType ReasonType;

private:
    typedef internal::EachChild<EachBrick<GeneratorT>> EachChildType;
    typedef BrickPtr<ResultType, ReasonType> BrickPtrType;
public:
    template<typename... ItemsT>
    EachBrick(Inplace<ItemsT...> args, std::size_t limit):
        generator(ItemsT::get(args)...),
        limit(limit),
        successor(nullptr) {}

    void start(Successor & successor) {
        this->successor = &successor;
        if (!this->generator) {
            this->brick = makeBrick<SuccessBrick<void(), Und>>();
            this->successor->onUpdate();
            return;
        }

        for (std::size_t limit = this->limit; this->generator && limit; --limit) {
            this->children.insert(new EachChildType(this->generator()));
        }

        for (EachChildType * child : utils::firewalk(this->children)) {
            child->start(this);
        }
    }

    Status getStatus() const {
        return this->brick ? NEXT : PENDING;
    }

    BrickPtrType getNext() {
        return std::move(this->brick);
    }

    void abort() {}

private:
    void onChildSuccess(BrickPtrType brick);
    void onChildError(BrickPtrType brick);

    GeneratorT generator;
    std::size_t limit;
    utils::CordList<EachChildType> children;
    Successor * successor;
    BrickPtrType brick;

    friend EachChildType;
};

template<typename GeneratorT>
void EachBrick<GeneratorT>::onChildSuccess(BrickPtrType brick) {
    if (this->children.empty()) {
        if (!this->brick) {
            this->brick = std::move(brick);
        }
        this->successor->onUpdate();
    }
}

template<typename GeneratorT>
void EachBrick<GeneratorT>::onChildError(BrickPtrType brick) {
    if (!this->brick) {
        this->brick = std::move(brick);
    }
    if (this->children.empty()) {
        this->successor->onUpdate();
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_EACH_BRICK_H
