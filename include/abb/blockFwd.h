#ifndef ABB_BLOCK_FWD_H
#define ABB_BLOCK_FWD_H

#include <abb/value.h>

#include <functional>
#include <type_traits>

namespace abb {

template<typename ResultT, typename ReasonT>
class BaseBlock;

template<typename ResultT = Und, typename ReasonT = Und>
using Block = BaseBlock<NormalizeValue<ResultT>, NormalizeValue<ReasonT>>;

template<typename ArgT>
using GetBlock = BaseBlock<GetResult<ArgT>, GetReason<ArgT>>;

typedef Block<void()> VoidBlock;
typedef Block<> UndBlock;

} // namespace abb

#endif // ABB_BLOCK_FWD_H
