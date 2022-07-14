#include "Buffer.h"
#include "Types.h"

#include <iostream>

using namespace bamboo;
using namespace bamboo::net;

Buffer buffer;

void test() {
    string str("hello world, this is test context\r\nif correct, it will be spilit\nhah");
    buffer.append(str);
    std::cout << buffer.readableBytes() << std::endl;
    auto pos = buffer.findCRLF();
    std::cout << string(buffer.peek(), pos) << std::endl;
    buffer.retrieveUntil(pos + 2);
    
    std::cout << buffer.readableBytes() << std::endl;
    buffer.prependInt8(static_cast<int8_t>(buffer.readableBytes() + 10));
    int len = buffer.peekInt8();
    std::cout << len << std::endl;
    std::cout << buffer.readInt8() << std::endl;
    std::cout << string(buffer.peek(), buffer.readableBytes()) << std::endl;

    pos = buffer.findEOL();
    std::cout << string(buffer.peek(), pos) << std::endl;
    buffer.retrieveUntil(pos + 1);
    std::cout << buffer.readableBytes() << std::endl;
    std::cout << string(buffer.peek(), buffer.readableBytes()) << std::endl;

}

int main() {
    test();

    return 0;
}