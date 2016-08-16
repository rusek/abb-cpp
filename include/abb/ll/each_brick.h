#ifndef ABB_LL_EACH_BRICK_H
#define ABB_LL_EACH_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/success_brick.h>
#include <abb/utils/noncopyable.h>
#include <abb/utils/cord.h>
#include <abb/utils/firewalk.h>
#include <abb/utils/range.h>

namespace abb {
namespace ll {

template<typename Range>
class range_generator : utils::noncopyable {
private:
    typedef decltype(utils::begin(std::declval<Range&>())) iterator;
    typedef decltype(*std::declval<iterator>()) value_type;

public:
    explicit range_generator(Range && range):
        range(std::forward<Range>(range)),
        it(utils::begin(this->range)),
        end(utils::end(this->range)) {}

    explicit range_generator(Range const& range):
        range(range),
        it(utils::begin(this->range)),
        end(utils::end(this->range)) {}

    explicit operator bool() const {
        return this->it != this->end;
    }

    value_type operator()() {
        value_type elem = *this->it;
        ++this->it;
        return std::forward<value_type>(elem);
    }

private:
    Range range;
    iterator it;
    iterator end;
};

template<typename Iterator>
class iterator_generator {
private:
    typedef decltype(*std::declval<Iterator>()) value_type;

public:
    iterator_generator(Iterator begin, Iterator end):
        it(begin),
        end(end) {}

    explicit operator bool() const {
        return this->it != this->end;
    }

    value_type operator()() {
        value_type elem = *this->it;
        ++this->it;
        return std::forward<value_type>(elem);
    }

private:
    Iterator it;
    Iterator end;
};

template<typename Generator, typename Func>
class map_generator {
private:
    box<Generator> wrapped;
    box<Func> func;

public:
    template<typename GeneratorArg, typename FuncArg>
    map_generator(GeneratorArg && wrapped, FuncArg && func):
        wrapped(box_arg, std::forward<GeneratorArg>(wrapped)),
        func(box_arg, std::forward<FuncArg>(func)) {}

    explicit operator bool() const {
        return bool(*this->wrapped);
    }

    auto operator()() -> decltype((*this->func)((*this->wrapped)())) {
        return (*this->func)((*this->wrapped)());
    }
};

namespace internal {

template<typename EachBrick>
class each_child : private successor, public utils::cord_node {
private:
    typedef get_brick_ptr_t<EachBrick> brick_ptr_type;

public:
    each_child(brick_ptr_type in_brick, EachBrick * parent):
        in_brick(std::move(in_brick)),
        parent(parent) {}

private:
    virtual void on_update();
    virtual island & get_island() const;
    virtual bool is_aborted() const;

    brick_ptr_type in_brick;
    EachBrick * parent;

    friend EachBrick;
};

template<typename EachBrick>
void each_child<EachBrick>::on_update() {
    EachBrick * parent = this->parent;
    status parent_status = parent->update_child(this);
    if (parent_status != status::running) {
        parent->succ->on_update();
    }
}

template<typename EachBrick>
island & each_child<EachBrick>::get_island() const {
    return this->parent->succ->get_island();
}

template<typename EachBrick>
bool each_child<EachBrick>::is_aborted() const {
    return this->parent->out_brick || this->parent->succ->is_aborted();
}

} // namespace internal

template<typename Generator>
class each_brick : public brick<void(), typename decltype(std::declval<Generator>()())::reason> {
public:
    typedef void result();
    typedef typename decltype(std::declval<Generator>()())::reason reason;

private:
    typedef internal::each_child<each_brick<Generator>> each_child_type;
    typedef brick_ptr<result, reason> brick_ptr_type;
public:
    template<typename GeneratorArg>
    each_brick(GeneratorArg && arg, std::size_t limit):
        generator(box_arg, std::forward<GeneratorArg>(arg)),
        limit(limit),
        succ(nullptr) {}

    void start(successor & succ) {
        this->succ = &succ;
        if (!*this->generator) {
            this->out_brick = make_brick<success_brick<void()>>();
            return;
        }

        for (std::size_t limit = this->limit; *this->generator && limit; --limit) {
            this->children.insert(new each_child_type((*this->generator)(), this));
        }

        for (each_child_type * child : utils::firewalk(this->children)) {
            this->update_child(child);
        }
    }

    void adopt(successor & succ) {
        this->succ = &succ;
    }

    status get_status() const {
        if (this->out_brick && this->children.empty()) {
            return status::next;
        } else if (this->succ) {
            return status::running;
        } else {
            return status::startable;
        }
    }

    brick_ptr_type get_next() {
        return std::move(this->out_brick);
    }

    void abort() {
        // TODO implement
    }

private:
    status update_child(each_child_type * child);

    box<Generator> generator;
    std::size_t limit;
    utils::cord_list<each_child_type> children;
    successor * succ;
    brick_ptr_type out_brick;

    friend each_child_type;
};

template<typename Generator>
status each_brick<Generator>::update_child(each_child_type * child) {
    for (;;) {
        status in_status = child->in_brick.try_start(*child);
        if (in_status == status::success && *this->generator) { // TODO stop if out_brick is set
            child->in_brick = (*this->generator)();
        } else {
            brick_ptr_type in_brick = std::move(child->in_brick);
            child->cord_detach();
            delete child;

            if (in_status == status::success) {
                if (this->children.empty()) {
                    if (!this->out_brick) {
                        this->out_brick = std::move(in_brick);
                    }
                    return status::next;
                }
            } else {
                if (!this->out_brick) {
                    this->out_brick = std::move(in_brick);
                    // TODO issue abort
                }
                if (this->children.empty()) {
                    return status::next;
                }
            }
            return status::running;
        }
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_EACH_BRICK_H
