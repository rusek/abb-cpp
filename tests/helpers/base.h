#ifndef TESTS_HELPERS_BASE
#define TESTS_HELPERS_BASE

#include <abb.h>

#include <iostream>
#include <cstdlib>

#define LOG(msg) do { std::cerr << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; } while (0)
#define ARG(...) __VA_ARGS__

#define RUN_FUNCTION(func) \
    do { \
        LOG("RUN_FUNCTION(" << #func << ")"); \
        abb::Island island; \
        island.enqueueExternal(&(func)); \
        island.run(); \
    } while (0)

#define RUN_CLASS(cls) \
    do { \
        LOG("RUN_CLASS(" << #cls << ")"); \
        abb::Island island; \
        cls obj; \
        island.enqueueExternal(std::ref(obj)); \
        island.run(); \
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

#endif // TESTS_HELPERS_BASE
