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
    void expectHits(std::uint32_t expectedHits);

    static HitCounter & current();

private:
    std::uint32_t hits;
    std::uint32_t expectedHits;

    static HitCounter * currentPtr;
};

#define HIT() (HitCounter::current().hit())
#define EXPECT_HITS(num) (HitCounter::current().expectHits(num))

struct Runner {
    void run() {
        this->island.run();
    }

    HitCounter hitCounter;
    abb::Island island;
};

template<typename FuncT, typename ReturnT>
struct FunctionRunner {};

template<typename FuncT>
struct FunctionRunner<FuncT, void> : Runner {
    void run(FuncT func) {
        this->island.enqueueExternal(func);
        this->Runner::run();
    }
};

template<typename FuncT>
struct FunctionRunner<FuncT, abb::Block<void>> : Runner {
    void run(FuncT func) {
        this->island.enqueueExternal(std::bind(&FunctionRunner::runAndEnqueue, this, func));
        this->Runner::run();
    }

private:
    void runAndEnqueue(FuncT func) {
        this->island.enqueue(func());
    }
};

template<typename FuncT>
void runFunction(FuncT func) {
    FunctionRunner<FuncT, typename std::result_of<FuncT()>::type>().run(func);
}

template<typename ClassT>
void runClass() {
    typedef std::reference_wrapper<ClassT> FuncT;
    FunctionRunner<FuncT, typename std::result_of<FuncT()>::type> runner;
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

template<typename ValueT>
struct IgnoreValueImpl {};

template<typename... ArgsT>
struct IgnoreValueImpl<void(ArgsT...)> {
    struct Type {
        abb::VoidBlock operator()(ArgsT...) {
            return abb::success();
        }
    };
};

template<>
struct IgnoreValueImpl<abb::Und> {
    typedef abb::Und Type;
};

template<typename BlockT>
using IgnoreResult = typename IgnoreValueImpl<abb::GetResult<BlockT>>::Type;

template<typename BlockT>
using IgnoreReason = typename IgnoreValueImpl<abb::GetReason<BlockT>>::Type;

#endif // TESTS_HELPERS_BASE_H
