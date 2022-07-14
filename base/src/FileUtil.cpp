#include "FileUtil.h"
#include "Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

using namespace bamboo::fileutil;

void AppendFile::append(const char* str, int len) {
    int written = 0;
    while (written != len) {
        int remain = len - written;
        int n = write(str, remain);
        if (n != remain) {
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed is %s\n", strerror_tl(err));
                break;
            }
        }
        written += n;
    }
    writtenBytes_ += written;
}

void AppendFile::flush() {
    ::fflush(fp_);
}

int AppendFile::write(const char* str, int len) {
    return static_cast<int>(fwrite_unlocked(str, 1, len, fp_));
}