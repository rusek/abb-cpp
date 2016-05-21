#ifndef ABB_UTILS_DEBUG_H
#define ABB_UTILS_DEBUG_H

#include <cstdint>
#include <sstream>

namespace abb {
namespace utils {
namespace internal {

struct FiascoInfo {
    const char * file;
    std::uint32_t line;
    const char * msg;
};

[[noreturn]] void fiasco(FiascoInfo const& info);
void trace(char const* file, std::uint32_t line, char const* msg);

#if 1
#define ABB_TRACE(msg) \
    do { \
        ::std::stringstream __ss; \
        __ss << msg; \
        ::abb::utils::internal::trace(__FILE__, __LINE__, __ss.str().c_str()); \
    } \
    while (0)
#else
#define ABB_TRACE(msg) do {} while (0)
#endif

#define ABB_FIASCO(msg) \
    do { \
        static const ::abb::utils::internal::FiascoInfo __fiascoInfo = { \
            __FILE__, \
            __LINE__, \
            (msg) \
        }; \
        ::abb::utils::internal::fiasco(__fiascoInfo); \
    } while (0)

#define ABB_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            ABB_FIASCO(msg); \
        } \
    } while (0)

} // namespace internal
} // namespace utils
} // namespace abb

#endif // ABB_UTILS_DEBUG_H
