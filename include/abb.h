#ifndef ABB_H
#define ABB_H

#include <abb/island.h>
#include <abb/ll/successBrick.h>
#include <abb/block.h>

namespace abb {

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

private:
    ll::ValueBrick<void(Args...), Und> & brick;
};

} // namespace internal

template<typename BlockType, typename FuncType>
BlockType impl(FuncType func) {
    typedef ll::ValueBrick<typename BlockType::ResultType, Und> BrickType;
    typedef Answer<typename BlockType::ResultType> AnswerType;

    BrickType * brick = new BrickType;
    try {
        AnswerType * answer = new internal::AnswerImpl<typename BlockType::ResultType>(*brick);
        func(*answer);
    } catch (...) {
        delete brick;
        throw;
    }

    return BlockType(std::unique_ptr<typename BrickType::BaseType>(brick));
}

template<typename FuncType>
auto run(FuncType func) -> decltype(func()) {
    FuncType * func2 = new FuncType(std::move(func));

    return (*func2)().exit(std::bind(std::default_delete<FuncType>(), func2));
}

} // namespace abb

#endif // ABB_H
