#include <abb/utils/debug.h>

#include <iostream>
#include <cstdlib>

namespace abb {
namespace utils {
namespace internal {

void fiasco(FiascoInfo const& info) {
    std::cerr << "File " << info.file << ", line " << info.line << ": " << info.msg << std::endl;
    std::abort();
}

} // namespace internal
} // namespace utils
} // namespace abb
