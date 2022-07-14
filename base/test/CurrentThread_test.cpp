#include "CurrentThread.h"
#include "Thread.h"

#include <iostream>

using namespace bamboo;

int main() {
    currentthread::tid();
    std::cout << currentthread::t_cachedTid << std::endl;
    std::cout << currentthread::t_tidString << std::endl;
    std::cout << currentthread::t_tidStringLength << std::endl;
    std::cout << currentthread::stackTrace(true) << std::endl;

    return 0;
}