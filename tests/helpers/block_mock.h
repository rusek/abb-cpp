#ifndef TESTS_HELPERS_BLOCK_MOCK_H
#define TESTS_HELPERS_BLOCK_MOCK_H

template<typename BlockResult = abb::und_t, typename BlockReason = abb::und_t>
class block_mock {
public:
    typedef abb::block<BlockResult, BlockReason> block_type;
    typedef abb::get_reply_t<block_type> reply_type;
    typedef std::function<void()> action_type;

    block_mock();

    block_type get();

    template<typename... Args>
    void set_result(Args &&... args) const;

    template<typename... Args>
    void enqueue_set_result(Args &&... args) const;

    void set_aborted() const;

    void enqueue_set_aborted() const;

    bool is_aborted() const;

    void expect_start(action_type action = &nop) const;

    void expect_abort(action_type action = &nop) const;

    template<typename Func>
    void enqueue(Func && func) const;

private:
    struct state {
        reply_type reply;
        action_type start;
        action_type abort;
    };

    struct worker {
        explicit worker(std::shared_ptr<state> shared_state): shared_state(shared_state) {}

        std::function<void()> operator()(reply_type reply) {
            this->shared_state->reply = std::move(reply);
            fire_action(this->shared_state->start, "start");
            return std::bind(&worker::abort, this);
        }

        void abort() {
            fire_action(this->shared_state->abort, "abort");
        }

        std::shared_ptr<state> shared_state;
    };

    static void nop() {}

    static void fire_action(action_type & action, const char * name) {
        action_type extracted_action;
        extracted_action.swap(action);
        if (!extracted_action) {
            FAILURE("No action set for " << name);
        }
        extracted_action();
    }

    std::shared_ptr<state> shared_state;
};

template<typename BlockResult, typename BlockReason>
block_mock<BlockResult, BlockReason>::block_mock(): shared_state(new state) {}

template<typename BlockResult, typename BlockReason>
typename block_mock<BlockResult, BlockReason>::block_type block_mock<BlockResult, BlockReason>::get() {
    return abb::impl<block_type>(worker(this->shared_state));
}

template<typename BlockResult, typename BlockReason>
template<typename... Args>
void block_mock<BlockResult, BlockReason>::set_result(Args &&... args) const {
    this->shared_state->reply.set_result(std::forward<Args>(args)...);
}

template<typename BlockResult, typename BlockReason>
template<typename... Args>
void block_mock<BlockResult, BlockReason>::enqueue_set_result(Args &&... args) const {
    this->enqueue(
        abb::partial(
            &block_mock<BlockResult, BlockReason>::set_result<Args...>,
            *this,
            std::forward<Args>(args)...
        )
    );
}

template<typename BlockResult, typename BlockReason>
void block_mock<BlockResult, BlockReason>::set_aborted() const {
    this->shared_state->reply.set_aborted();
}

template<typename BlockResult, typename BlockReason>
void block_mock<BlockResult, BlockReason>::enqueue_set_aborted() const {
    this->enqueue(
        abb::partial(
            &block_mock<BlockResult, BlockReason>::set_aborted,
            *this
        )
    );
}

template<typename BlockResult, typename BlockReason>
bool block_mock<BlockResult, BlockReason>::is_aborted() const {
    return this->shared_state->reply.is_aborted();
}

template<typename BlockResult, typename BlockReason>
void block_mock<BlockResult, BlockReason>::expect_start(action_type action) const {
    this->shared_state->start = action;
}

template<typename BlockResult, typename BlockReason>
void block_mock<BlockResult, BlockReason>::expect_abort(action_type action) const {
    this->shared_state->abort = action;
}

template<typename BlockResult, typename BlockReason>
template<typename Func>
void block_mock<BlockResult, BlockReason>::enqueue(Func && func) const {
    this->shared_state->reply.get_island().enqueue(std::forward<Func>(func));
}

#endif // TESTS_HELPERS_BLOCK_MOCK_H
