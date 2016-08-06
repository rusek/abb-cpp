#ifndef TESTS_HELPERS_BASE_H
#define TESTS_HELPERS_BASE_H

#include <abb.h>

#include <iostream>
#include <cstdlib>

#define LOG(msg) do { std::cerr << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; } while (0)

class HitCounter {
public:
    HitCounter();
    ~HitCounter();

    void hit();
    void hit(std::uint32_t hitIndex);
    void expectHits(std::uint32_t expectedHits);

    static HitCounter & current();

private:
    std::uint32_t hits;
    std::uint32_t expectedHits;

    static HitCounter * current_ptr;
};

#define HIT(...) (HitCounter::current().hit(__VA_ARGS__))
#define EXPECT_HITS(num) (HitCounter::current().expectHits(num))

struct Runner {
    void run() {
        this->island.run();
    }

    HitCounter hitCounter;
    abb::island island;
};

template<typename Func, typename Return>
struct FunctionRunner {};

template<typename Func>
struct FunctionRunner<Func, void> : Runner {
    void run(Func func) {
        this->island.enqueue_external(func);
        this->Runner::run();
    }
};

template<typename Func>
struct FunctionRunner<Func, abb::block<void>> : Runner {
    void run(Func func) {
        this->island.enqueue_external(std::bind(&FunctionRunner::runAndEnqueue, this, func));
        this->Runner::run();
    }

private:
    void runAndEnqueue(Func func) {
        this->island.enqueue(func());
    }
};

template<typename Func, typename Return>
struct FunctionRunnerWithIsland;

template<typename Func>
struct FunctionRunnerWithIsland<Func, void> : Runner {
    void run(Func func) {
        this->island.enqueue_external(std::bind(func, std::ref(this->island)));
        this->Runner::run();
    }
};

template<typename Func>
auto runFunction(Func func) -> decltype(func(), void()) {
    FunctionRunner<Func, typename std::result_of<Func()>::type>().run(func);
}

template<typename Func>
auto runFunction(Func func) -> decltype(func(std::declval<abb::island&>()), void()) {
    FunctionRunnerWithIsland<Func, decltype(func(std::declval<abb::island&>()))>().run(func);
}

template<typename ClassT>
void runClass() {
    typedef std::reference_wrapper<ClassT> Func;
    FunctionRunner<Func, typename std::result_of<Func()>::type> runner;
    ClassT obj;
    runner.run(std::ref(obj));
}

#define RUN_FUNCTION(...) \
    do { \
        LOG("RUN_FUNCTION(" << #__VA_ARGS__ << ")"); \
        runFunction(__VA_ARGS__); \
    } while (0)

#define RUN_CLASS(...) \
    do { \
        LOG("RUN_CLASS(" << #__VA_ARGS__ << ")"); \
        runClass<__VA_ARGS__>(); \
    } while (0)

#define FAILURE(msg) \
    do { \
        LOG(msg); \
        ::abort(); \
    } while (0)

#define REQUIRE(cond) \
    do { \
        if (!(cond)) { \
            FAILURE("Check " << #cond << " failed"); \
        } \
    } while (0)

#define REQUIRE_EQUAL(left, right) \
    do { \
        if (!((left) == (right))) { \
            FAILURE("Check (" << #left << ") == (" << right << ") failed, left=" << (left) << ", right=" << (right)); \
        } \
    } while (0)

template<typename Value>
struct IgnoreValueImpl {};

template<typename... Args>
struct IgnoreValueImpl<void(Args...)> {
    struct type {
        abb::void_block operator()(Args...) {
            return abb::success();
        }
    };
};

template<>
struct IgnoreValueImpl<abb::und_t> {
    typedef abb::und_t type;
};

template<typename Block>
using IgnoreResult = typename IgnoreValueImpl<abb::get_result_t<Block>>::type;

template<typename Block>
using IgnoreReason = typename IgnoreValueImpl<abb::get_reason_t<Block>>::type;

#endif // TESTS_HELPERS_BASE_H
