#include "Timestamp.h"
#include <iostream>

using namespace bamboo;


int main() {
    Timestamp ts(Timestamp::now());

    std::cout << ts.toString() << std::endl;
    std::cout << ts.toFormattedString() << std::endl;

    if (ts.valid()) {
        std::cout << ts.microSecondsSinceEpoch() << std::endl;
    }
    return 0;
}