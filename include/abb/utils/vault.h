#ifndef ABB_UTILS_VAULT_H
#define ABB_UTILS_VAULT_H

#include <type_traits>

namespace abb {
namespace utils {

template<typename ValueT>
class Vault {
public:
    template<typename... ArgsT>
    void init(ArgsT &&... args) {
        new (&this->storage) ValueT(std::forward<ArgsT>(args)...);
    }

    ValueT & operator*() {
        return *reinterpret_cast<ValueT*>(&this->storage);
    }

    void destroy() {
        reinterpret_cast<ValueT*>(&this->storage)->~ValueT();
    }

private:
    typename std::aligned_storage<sizeof(ValueT), alignof(ValueT)>::type storage;
};

} // namespace utils
} // namespace abb

#endif // ABB_UTILS_VAULT_H
