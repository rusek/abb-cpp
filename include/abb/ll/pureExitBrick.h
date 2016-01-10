#ifndef ABB_LL_PURE_EXIT_BRICK_H
#define ABB_LL_PURE_EXIT_BRICK_H

#include <abb/ll/brick.h>

namespace abb {
namespace ll {

template<typename Result, typename Reason>
class PureExitBrick {};

template<typename... Args>
class PureExitBrick<void(Args...), Und> : public Brick<void(Args...), Und>, private Successor<void(Args...), Und> {
public:
    typedef Brick<void(Args...), Und> BaseType;
    typedef Successor<void(Args...), Und> SuccessorType;

    PureExitBrick(std::function<void()> onexit, std::unique_ptr<BaseType> brick);

    virtual ~PureExitBrick();

    virtual void setSuccessor(SuccessorType & successor);

private:
    virtual void onsuccess(Args...);

    std::function<void()> onexit;
    std::unique_ptr<BaseType> brick;
    SuccessorType * successor;
};

template<typename... Args>
PureExitBrick<void(Args...), Und>::PureExitBrick(std::function<void()> onexit, std::unique_ptr<BaseType> brick):
        onexit(onexit),
        brick(std::move(brick)),
        successor(nullptr) {
}

template<typename... Args>
PureExitBrick<void(Args...), Und>::~PureExitBrick() {
}

template<typename... Args>
void PureExitBrick<void(Args...), Und>::setSuccessor(SuccessorType & successor) {
    this->successor = &successor;
    this->brick->setSuccessor(*this);
}

template<typename... Args>
void PureExitBrick<void(Args...), Und>::onsuccess(Args... args) {
    this->onexit();
    this->successor->onsuccess(args...);
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PURE_EXIT_BRICK_H
