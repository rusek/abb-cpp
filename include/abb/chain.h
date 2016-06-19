#ifndef ABB_CHAIN_H
#define ABB_CHAIN_H

#include <abb/block.h>

namespace abb {

template<typename ResultT, typename ReasonT>
inline BaseBlock<ResultT, ReasonT> chain(BaseBlock<ResultT, ReasonT> && block) {
    return std::move(block);
}

template<typename ResultT, typename ReasonT, typename ContT, typename... ContsT>
inline auto chain(BaseBlock<ResultT, ReasonT> && block, ContT && cont, ContsT &&... conts) ->
    decltype(chain(std::move(block).pipe(std::forward<ContT>(cont)), std::forward<ContsT>(conts)... ))
{
    return chain(std::move(block).pipe(std::forward<ContT>(cont)), std::forward<ContsT>(conts)... );
}

} // namespace abb

#endif // ABB_CHAIN_H
