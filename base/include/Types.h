#ifndef BAMBOO_BASE_TYPES_H
#define BAMBOO_BASE_TYPES_H

#include <cstring>
#include <string>
#include <cstdint>

namespace bamboo
{
using std::string;

template<class To, class From>
To implicit_cast(From f) {
    return f;
}

}
#endif