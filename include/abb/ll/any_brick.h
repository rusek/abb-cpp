#ifndef ABB_LL_ANY_BRICK_H
#define ABB_LL_ANY_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>
#include <abb/ll/abort_brick.h>
#include <abb/utils/cord.h>
#include <abb/utils/firewalk.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class any_brick;

namespace internal {

template<typename Result, typename Reason>
class any_child : private successor, public utils::cord_node {
private:
    typedef any_brick<Result, Reason> any_brick_type;
    typedef any_child<Result, Reason> any_child_type;
    typedef brick_ptr<Result, Reason> brick_ptr_type;
public:
    explicit any_child(brick_ptr_type && in_brick):
        parent(nullptr),
        in_brick(std::move(in_brick)) {}

    void start(any_brick_type * parent);

    void abort();

    virtual void on_update();
    virtual island & get_island() const;
    virtual bool is_aborted() const;

    any_brick_type * parent;
    brick_ptr_type in_brick;
};

template<typename Result, typename Reason>
void any_child<Result, Reason>::start(any_brick_type * parent) {
    this->parent = parent;
    this->on_update();
}

template<typename Result, typename Reason>
void any_child<Result, Reason>::abort() {
    if (this->parent) {
        this->in_brick.abort();
    }
}

template<typename Result, typename Reason>
void any_child<Result, Reason>::on_update() {
    for (;;) {
        status cur_status = this->in_brick.get_status();
        if (cur_status == pending_status) {
            /*
            any_brick_type * any_brick = brick_cast<any_brick_type>(this->in_brick);
            if (any_brick) {
                // FIXME not enough, we should try also any_brick<Result, und_t>, any_brick<und_t, Reason>
                // and any_brick<und_t, und_t>
                ABB_FIASCO("not implemented");
            } else {
                this->in_brick.start(*this);
            }
            */
            this->in_brick.start(*this);
            return;
        } else if (cur_status & next_status) {
            this->in_brick = this->in_brick.get_next();
        } else {
            this->cord_detach();
            any_brick_type * parent = this->parent;
            brick_ptr_type in_brick = std::move(this->in_brick);
            delete this;
            parent->on_child_finish(std::move(in_brick));
            return;
        }
    }
}

template<typename Result, typename Reason>
island & any_child<Result, Reason>::get_island() const {
    return this->parent->succ->get_island();
}

template<typename Result, typename Reason>
bool any_child<Result, Reason>::is_aborted() const {
    return this->parent->out_brick || this->parent->succ->is_aborted();
}

} // namespace internal

template<typename Result, typename Reason>
class any_brick : public brick<Result, Reason> {
private:
    typedef internal::any_child<Result, Reason> any_child_type;
    typedef brick_ptr<Result, Reason> brick_ptr_type;
public:
    template<typename Iterator>
    any_brick(Iterator begin, Iterator end):
        succ(nullptr)
    {
        for (; begin != end; ++begin) {
            this->children.insert(new any_child_type(std::move(*begin)));
        }
    }

    any_brick():
        succ(nullptr) {}

    ~any_brick() {
        for (any_child_type * child : utils::firewalk(this->children)) {
            delete child;
        }
    }

    void start(successor & succ) {
        this->succ = &succ;
        for (any_child_type * child : utils::firewalk(this->children)) {
            child->start(this);
        }
    }

    void abort() {
        if (!this->out_brick && !this->children.empty()) {
            for (any_child_type * child : utils::firewalk(this->children)) {
                child->abort();
            }
        } else {
            this->out_brick = make_brick<abort_brick>();
            this->succ->on_update();
        }
    }

    status get_status() const {
        return this->out_brick ? next_status : pending_status;
    }

    brick_ptr_type get_next() {
        return std::move(this->out_brick);
    }

private:
    void on_child_finish(brick_ptr_type out_brick);

    utils::cord_list<any_child_type> children;
    brick_ptr_type out_brick;
    successor * succ;

    friend any_child_type;
};

template<typename Result, typename Reason>
void any_brick<Result, Reason>::on_child_finish(brick_ptr_type out_brick) {
    if (this->out_brick) {
        out_brick.reset();
        if (this->children.empty()) {
            this->succ->on_update();
        }
    } else {
        this->out_brick = std::move(out_brick);
        if (this->children.empty()) {
            this->succ->on_update();
        } else {
            for (any_child_type * child : utils::firewalk(this->children)) {
                child->abort();
            }
        }
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_ANY_BRICK_H
