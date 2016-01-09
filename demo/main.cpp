#include <abb.h>

#define LOG(msg) do { std::cerr << "File " << __FILE__ << ", line " << __LINE__ << ": " << msg << std::endl; } while(0)

typedef abb::Block<void(int)> IntBlock;

IntBlock increment(int val) {
    LOG("increment(" << val << ")");
    return abb::success(val + 1);
}

void doSth() {
    IntBlock myBlock = abb::success(12).pipe<IntBlock>(&increment).pipe<IntBlock>(&increment);

    myBlock.detach();
}

int main() {
    abb::Island island;
    island.enqueue(&doSth);
    island.run();

    return 0;
}
