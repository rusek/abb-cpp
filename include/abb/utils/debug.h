#ifndef ABB_UTILS_DEBUG_H
#define ABB_UTILS_DEBUG_H

#include <iostream>
#include <cstdlib>

#define ABB_FIASCO(msg) \
    do { \
        std::cerr << "File " << __FILE__ << ", line " << __LINE__ << ": " << msg << std::endl; \
        std::abort(); \
    } while (0)

#define ABB_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            ABB_FIASCO(msg); \
        } \
    } while (0)

#endif // ABB_UTILS_DEBUG_H
