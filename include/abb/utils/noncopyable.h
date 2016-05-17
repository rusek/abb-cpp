#ifndef ABB_UTILS_NONCOPYABLE_H
#define ABB_UTILS_NONCOPYABLE_H

namespace abb {
namespace utils {

class Noncopyable {
public:
    Noncopyable() {}

    Noncopyable(Noncopyable const&) = delete;
    Noncopyable& operator=(Noncopyable const&) = delete;
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_NONCOPYABLE_H
