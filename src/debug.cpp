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

void trace(char const* file, std::uint32_t line, char const* msg) {
    std::cerr << "File " << file << ", line " << line << ": " << msg << std::endl;
}

} // namespace internal
} // namespace utils
} // namespace abb
