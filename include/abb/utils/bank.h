#ifndef ABB_UTILS_BANK_H
#define ABB_UTILS_BANK_H

#include <type_traits>
#include <utility>

namespace abb {
namespace utils {

template<typename Value>
class bank {
public:
    template<typename... Args>
    void init(Args &&... args) {
        new (&this->storage) Value(std::forward<Args>(args)...);
    }

    Value & operator*() {
        return *reinterpret_cast<Value*>(&this->storage);
    }

    void destroy() {
        reinterpret_cast<Value*>(&this->storage)->~Value();
    }

private:
    typename std::aligned_storage<sizeof(Value), alignof(Value)>::type storage;
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_BANK_H
