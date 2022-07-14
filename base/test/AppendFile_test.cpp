#include "FileUtil.h"
#include "Types.h"

using namespace bamboo;
using namespace bamboo::fileutil;

void bench(AppendFile& file) {
    const int knum = 1000;
    string s(1000, 'X');
    for (int i = 0; i < knum; ++i) {
        file.append(s.c_str(), s.size());
    }
    // file.flush();
}

int main() {
    AppendFile file("/home/xing/testfile");
    bench(file);
}