#ifndef BAMBOO_BASE_FILEUTIL_H
#define BAMBOO_BASE_FILEUTIL_H

// #include "LogStream.h" // for buffer, use setbuffer(fp, buf, sizeof buf)
#include <stdio.h>
#include <cassert>
#include "Types.h"

namespace bamboo
{

    namespace fileutil
    {

        class AppendFile
        {
        public:
            explicit AppendFile(const string& str)
                : fp_(::fopen(str.c_str(), "ae")), writtenBytes_(0)
            {
                assert(fp_);
                ::setbuffer(fp_, buf_, sizeof buf_);
            }

            void append(const char* str, int len);

            void flush();

            int writtenBytes() const { return writtenBytes_; }

            ~AppendFile() { ::fclose(fp_); }

        private:
            int write(const char* str, int len);

            // size 64k
            char buf_[64 * 1024];
            FILE *fp_;
            // how much bytes file has written
            int writtenBytes_;
        };

    } // detail

} // bamboo

#endif