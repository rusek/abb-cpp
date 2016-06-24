#ifndef ABB_BLOCK_FWD_H
#define ABB_BLOCK_FWD_H

#include <abb/value.h>

#include <functional>
#include <type_traits>

namespace abb {

template<typename Result, typename Reason>
class base_block;

template<typename Result = und_t, typename Reason = und_t>
using block = base_block<normalize_value_t<Result>, normalize_value_t<Reason>>;

template<typename Arg>
using get_block_t = base_block<get_result_t<Arg>, get_reason_t<Arg>>;

typedef block<void()> void_block;
typedef block<> und_block;

} // namespace abb

#endif // ABB_BLOCK_FWD_H
