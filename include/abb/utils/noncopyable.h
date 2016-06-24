#ifndef ABB_UTILS_NONCOPYABLE_H
#define ABB_UTILS_NONCOPYABLE_H

namespace abb {
namespace utils {

class noncopyable {
public:
    noncopyable() {}

    noncopyable(noncopyable const&) = delete;
    noncopyable& operator=(noncopyable const&) = delete;
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_NONCOPYABLE_H
