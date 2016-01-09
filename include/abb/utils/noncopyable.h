#ifndef ABB_UTILS_H
#define ABB_UTILS_H

namespace abb {
namespace utils {

class Noncopyable {
public:
    Noncopyable() {}

private:
    Noncopyable(Noncopyable const&);
    void operator=(Noncopyable const&);
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_H