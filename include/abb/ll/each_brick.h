#ifndef ABB_LL_EACH_BRICK_H
#define ABB_LL_EACH_BRICK_H

#include <abb/inplace.h>
#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/store.h>
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
    Generator wrapped;
    Func func;

public:
    template<typename... GeneratorItems, typename... FuncItems>
    map_generator(inplace_tuple<GeneratorItems...> wrapped_args, inplace_tuple<FuncItems...> func_args):
        wrapped(GeneratorItems::get(wrapped_args)...),
        func(FuncItems::get(func_args)...) {}

    explicit operator bool() const {
        return bool(this->wrapped);
    }

    auto operator()() -> decltype(this->func(this->wrapped())) {
        return this->func(this->wrapped());
    }
};

namespace internal {

template<typename EachBrick>
class each_child : private successor, public utils::cord_node {
private:
    typedef get_brick_ptr_t<EachBrick> brick_ptr_type;

public:
    explicit each_child(brick_ptr_type in_brick): in_brick(std::move(in_brick)) {}

    void start(EachBrick * parent);

private:
    virtual void on_update();
    virtual island & get_island() const;
    virtual bool is_aborted() const;

    brick_ptr_type in_brick;
    EachBrick * parent;
};

template<typename EachBrick>
void each_child<EachBrick>::start(EachBrick * parent) {
    this->parent = parent;
    this->on_update();
}

template<typename EachBrick>
void each_child<EachBrick>::on_update() {
    while (status in_status = this->in_brick.try_start(*this)) {
        if (in_status & success_status) {
            if (this->parent->generator) {
                this->in_brick = this->parent->generator();
            } else {
                brick_ptr_type in_brick = std::move(this->in_brick);
                EachBrick * parent = this->parent;
                this->cord_detach();
                delete this;
                parent->on_child_success(std::move(in_brick));
                return;
            }
        } else {
            brick_ptr_type in_brick = std::move(this->in_brick);
            EachBrick * parent = this->parent;
            this->cord_detach();
            delete this;
            parent->on_child_error(std::move(in_brick));
            return;
        }
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
    template<typename... Items>
    each_brick(inplace_tuple<Items...> args, std::size_t limit):
        generator(Items::get(args)...),
        limit(limit),
        succ(nullptr) {}

    void start(successor & succ) {
        this->succ = &succ;
        if (!this->generator) {
            this->out_brick = make_brick<success_brick<void()>>();
            this->succ->on_update();
            return;
        }

        for (std::size_t limit = this->limit; this->generator && limit; --limit) {
            this->children.insert(new each_child_type(this->generator()));
        }

        for (each_child_type * child : utils::firewalk(this->children)) {
            child->start(this);
        }
    }

    status get_status() const {
        return this->out_brick ? next_status : pending_status;
    }

    brick_ptr_type get_next() {
        return std::move(this->out_brick);
    }

    void abort() {}

private:
    void on_child_success(brick_ptr_type out_brick);
    void on_child_error(brick_ptr_type out_brick);

    Generator generator;
    std::size_t limit;
    utils::cord_list<each_child_type> children;
    successor * succ;
    brick_ptr_type out_brick;

    friend each_child_type;
};

template<typename Generator>
void each_brick<Generator>::on_child_success(brick_ptr_type out_brick) {
    if (this->children.empty()) {
        if (!this->out_brick) {
            this->out_brick = std::move(out_brick);
        }
        this->succ->on_update();
    }
}

template<typename Generator>
void each_brick<Generator>::on_child_error(brick_ptr_type out_brick) {
    if (!this->out_brick) {
        this->out_brick = std::move(out_brick);
    }
    if (this->children.empty()) {
        this->succ->on_update();
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_EACH_BRICK_H
