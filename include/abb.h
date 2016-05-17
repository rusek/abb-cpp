#ifndef ABB_H
#define ABB_H

#include <abb/island.h>
#include <abb/ll/valueBrick.h>
#include <abb/block.h>
#include <abb/error.h>
#include <abb/success.h>
#include <abb/impl.h>

namespace abb {

typedef Block<void()> VoidBlock;

template<typename Result>
class Answer {};

template<typename... Args>
class Answer<void(Args...)> {
public:
    virtual ~Answer() {}

    virtual void setResult(Args...) = 0;
};

namespace internal {

template<typename Result>
class AnswerImpl {};

template<typename... Args>
class AnswerImpl<void(Args...)> : public Answer<void(Args...)> {
public:
    explicit AnswerImpl(ll::ValueBrick<void(Args...), Und> & brick): brick(brick) {}

    virtual void setResult(Args... args) {
        this->brick.setResult(std::move(args)...);
        delete this;
    }

private:\
    ll::ValueBrick<void(Args...), Und> & brick;
};

} // namespace internal

template<typename BlockType, typename FuncType>
BlockType impl(FuncType func) {
    typedef ll::ValueBrick<typename BlockType::ResultType, Und> ValueBrickType;
    typedef Answer<typename BlockType::ResultType> AnswerType;

    ValueBrickType * brick = new ValueBrickType;
    try {
        AnswerType * answer = new internal::AnswerImpl<typename BlockType::ResultType>(*brick);
        func(*answer);
    } catch (...) {
        delete brick;
        throw;
    }

    return BlockType(ll::makeBrickPtr(brick));
}

} // namespace abb

#endif // ABB_H
