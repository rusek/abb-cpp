#ifndef ABB_LL_PURE_EXIT_BRICK_H
#define ABB_LL_PURE_EXIT_BRICK_H

#include <abb/ll/brick.h>

namespace abb {
namespace ll {

template<typename ResultT, typename ReasonT>
class PureExitBrick {};

template<typename... ArgsT>
class PureExitBrick<void(ArgsT...), Und> : public Brick<void(ArgsT...), Und>, private Successor<void(ArgsT...), Und> {
public:
    typedef Brick<void(ArgsT...), Und> BrickType;
    typedef typename BrickType::SuccessorType SuccessorType;

    PureExitBrick(std::function<void()> onexit, std::unique_ptr<BrickType> brick);

    virtual ~PureExitBrick();

    virtual void setSuccessor(SuccessorType & successor);

private:
    virtual void onsuccess(ArgsT...);

    std::function<void()> onexit;
    std::unique_ptr<BrickType> brick;
    SuccessorType * successor;
};

template<typename... ArgsT>
PureExitBrick<void(ArgsT...), Und>::PureExitBrick(std::function<void()> onexit, std::unique_ptr<BrickType> brick):
        onexit(onexit),
        brick(std::move(brick)),
        successor(nullptr) {
}

template<typename... ArgsT>
PureExitBrick<void(ArgsT...), Und>::~PureExitBrick() {
}

template<typename... ArgsT>
void PureExitBrick<void(ArgsT...), Und>::setSuccessor(SuccessorType & successor) {
    this->successor = &successor;
    this->brick->setSuccessor(*this);
}

template<typename... ArgsT>
void PureExitBrick<void(ArgsT...), Und>::onsuccess(ArgsT... args) {
    this->onexit();
    this->successor->onsuccess(args...);
}

} // namespace ll
} // namespace abb

#endif // ABB_LL_PURE_EXIT_BRICK_H
