#include <iostream>
#include <memory>
#include <utility>
#include <abb/utils/range.h>
#include <vector>


static int ott[3] = {1, 2, 7};

struct OTT {};

int * begin(OTT) {
    return ott;
}

int * end(OTT) {
    return ott + 3;
}


int main() {
    for (int x : OTT()) {
        std::cout << x << std::endl;
    }

    abb::utils::begin(OTT());

    return 0;
}