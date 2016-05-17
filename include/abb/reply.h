#ifndef ABB_REPLY_H
#define ABB_REPLY_H

#include <abb/blockFwd.h>
#include <abb/island.h>

namespace abb {

namespace internal {

template<typename ResultT, typename ReasonT>
struct Reply {
    typedef ResultT ResultType;
    typedef ReasonT ReasonType;
    typedef Block<ResultT, ReasonT> BlockType;

    virtual ~Reply() {}

    virtual Island & getIsland() const = 0;

//    virtual bool isAborted() const = 0;
//    virtual void setAborted() = 0;
};

template<typename ResultT, typename ReasonT>
struct SuccessReply : Reply<ResultT, ReasonT> {};

template<typename... ArgsT, typename ReasonT>
struct SuccessReply<void(ArgsT...), ReasonT> : Reply<void(ArgsT...), ReasonT> {
    virtual void setResult(ArgsT... args) = 0;
};

template<typename ResultT, typename ReasonT>
struct ErrorReply : SuccessReply<ResultT, ReasonT> {};

template<typename ResultT, typename... ArgsT>
struct ErrorReply<ResultT, void(ArgsT...)> : SuccessReply<ResultT, void(ArgsT...)> {
    virtual void setReason(ArgsT... args) = 0;
};

} // namespace internal

template<typename ResultT, typename ReasonT>
class BaseReply : public internal::ErrorReply<ResultT, ReasonT> {
public:
    typedef BaseReply<ResultT, ReasonT> ReplyType;
};

template<typename ResultT = Und, typename ReasonT = Und>
using Reply = BaseReply<internal::NormalizeValue<ResultT>, internal::NormalizeValue<ReasonT>>;

} // namespace abb

#endif // ABB_REPLY_H
