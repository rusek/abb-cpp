#ifndef ABB_CHAIN_H
#define ABB_CHAIN_H

#include <abb/block.h>

namespace abb {

template<typename Result, typename Reason>
inline base_block<Result, Reason> chain(base_block<Result, Reason> && block) {
    return std::move(block);
}

template<typename Result, typename Reason, typename Cont, typename... Conts>
inline auto chain(base_block<Result, Reason> && block, Cont && cont, Conts &&... conts) ->
    decltype(chain(std::move(block).pipe(std::forward<Cont>(cont)), std::forward<Conts>(conts)... ))
{
    return chain(std::move(block).pipe(std::forward<Cont>(cont)), std::forward<Conts>(conts)... );
}

} // namespace abb

#endif // ABB_CHAIN_H
