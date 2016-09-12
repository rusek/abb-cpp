#ifndef ABB_LL_ANY_BRICK_H
#define ABB_LL_ANY_BRICK_H

#include <abb/ll/brick.h>
#include <abb/ll/brick_ptr.h>
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

private:
    virtual void on_update();

    any_brick_type * parent;
    brick_ptr_type in_brick;

    friend any_brick_type;
};

template<typename Result, typename Reason>
void any_child<Result, Reason>::on_update() {
    any_brick_type * parent = this->parent;
    parent->update_child(this);
    if (parent->out_brick && parent->children.empty()) { // TODO test various false / true scenarios
        parent->succ->on_update();
    }
}

} // namespace internal

class hold_brick : public brick<und_t, und_t> {
public:
    hold_brick(): cur_status(status::startable) {}

    void start(island &, bool aborted, successor &) {
        this->cur_status = aborted ? status::abort : status::running;
    }

    void abort() {
        this->cur_status = status::abort;
    }

    void adopt(successor &) {} // TODO test

    status get_status() const {
        return this->cur_status;
    }

private:
    status cur_status;
};

template<typename Result, typename Reason>
class any_brick : public brick<Result, Reason> {
private:
    typedef internal::any_child<Result, Reason> any_child_type;
    typedef brick_ptr<Result, Reason> brick_ptr_type;
public:
    template<typename Iterator>
    any_brick(Iterator begin, Iterator end):
        target(nullptr),
        aborted(false),
        succ(nullptr)
    {
        for (; begin != end; ++begin) {
            this->children.insert(new any_child_type(std::move(*begin)));
        }

        if (this->children.empty()) {
            this->out_brick = make_brick<hold_brick>();
        }
    }

    any_brick():
        target(nullptr),
        aborted(false),
        succ(nullptr) {}

    ~any_brick() {
        for (any_child_type * child : utils::firewalk(this->children)) {
            delete child;
        }
    }

    void start(island & target, bool aborted, successor & succ) {
        this->target = &target;
        this->aborted = aborted;
        this->succ = &succ;
        for (any_child_type * child : utils::firewalk(this->children)) {
            child->parent = this;
            this->update_child(child);
        }
    }

    void adopt(successor & succ) { // TODO add test
        this->succ = &succ;
    }

    void abort() {
        if (!this->aborted) { // TODO add test
            this->aborted = true;
            for (any_child_type * child : utils::firewalk(this->children)) {
                this->abort_child(child);
            }
        }
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

private:
    void update_child(any_child_type * child);
    void abort_child(any_child_type * child);

    utils::cord_list<any_child_type> children;
    brick_ptr_type out_brick;
    island * target;
    bool aborted;
    successor * succ;

    friend any_child_type;
};

template<typename Result, typename Reason>
void any_brick<Result, Reason>::update_child(any_child_type * child) {
    if (child->in_brick.update(*this->target, this->aborted, *child) != status::running) {
        if (!this->out_brick) {
            this->out_brick = std::move(child->in_brick);
        }

        child->cord_detach();
        delete child;

        if (!this->aborted) {
            this->aborted = true;
            for (any_child_type * child : utils::firewalk(this->children)) {
                this->abort_child(child);
            }
        }
    }
}

template<typename Result, typename Reason>
void any_brick<Result, Reason>::abort_child(any_child_type * child) {
    if (child->parent) {
        child->in_brick.abort();
        this->update_child(child); // TODO test this call is necessary
    }
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_ANY_BRICK_H
