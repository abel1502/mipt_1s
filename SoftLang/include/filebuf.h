#ifndef FILEBUF_H
#define FILEBUF_H

#include "general.h"

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>


namespace SoftLang {

    class FileBuf {
    public:
        FACTORIES(FileBuf);

        bool ctor();

        /// File read constructor
        bool ctor(const char *name, const char *mode);

        /// Memory read constructor
        bool ctor(const char *src, size_t amount);

        /// Memory read constructor with implicit amount
        bool ctor(const char *src);

        void dtor();

        unsigned getSize() const;

        const char *getData() const;

        bool isInited() const;

    private:
        size_t size;
        char *buf;
    };


    class FileBufIterator {
    public:
        FACTORIES(FileBufIterator);

        bool ctor();

        bool ctor(const FileBuf *new_buf);

        void dtor();

        char cur() const;

        char peek(int offset) const;

        char next();

        char prev();

        void skipSpace();

        void trim();

        bool isEof() const;

        size_t getPos() const;

        const char *getPtr() const;

    private:
        const FileBuf *buf;
        size_t pos;

    };

}



#endif // FILEBUF_H
