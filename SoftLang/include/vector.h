#ifndef VECTOR_H
#define VECTOR_H

#include "general.h"

#include <cstdlib>
#include <cstring>
#include <cassert>


namespace SoftLang {

    template <typename T>
    class Vector {
    public:

        static const unsigned DEFAULT_CAPACITY = 2;

        FACTORIES(Vector<T>);

        bool ctor() {
            buf = nullptr;

            size = 0;
            capacity = 0;

            return false;
        }

        void dtor() {
            for (unsigned i = 0; i < size; ++i) {
                buf[i].dtor();
                buf[i] = {};
            }

            free(buf);
            buf = nullptr;

            size = 0;
            capacity = 0;
        }

        T &operator[](unsigned ind) {
            assert(size);
            ind = (ind + size) % size;

            assert(ind < size);

            assert(buf);

            return buf[ind];
        }

        const T &operator[](unsigned ind) const {
            assert(size);
            ind = (ind + size) % size;  // Despite being unsigned, ind would still work correctly if it's negative

            assert(ind < size);

            assert(buf);

            return buf[ind];
        }

        bool append(const T &value) {
            if (size >= capacity) {
                TRY_B(resize(capacity * 2));

                assert(capacity > size);
            }

            buf[size++] = value;

            return false;
        }

        const T &pop() {
            assert(!isEmpty());

            assert(buf);

            return buf[--size];
        }

        bool isEmpty() const {
            return size == 0;
        }

        unsigned getSize() const {
            return size;
        }

        unsigned getCapacity() const {
            return capacity;
        }

    private:

        T *buf;

        unsigned size;
        unsigned capacity;

        bool resize(unsigned new_capacity) {
            assert(new_capacity >= capacity);

            if (new_capacity == 0)
                new_capacity = DEFAULT_CAPACITY;

            T *newBuf = (T *)realloc(buf, new_capacity * sizeof(T));
            TRY_B(!newBuf);

            buf = newBuf;
            capacity = new_capacity;

            return false;
        }

    };


    template <typename T, unsigned COMPACT=1>
    class CompactVector {
    public:

        static_assert(sizeof(T) <= sizeof(void *));

        FACTORIES(CompactVector<T>);

        bool ctor() {
            buf = nullptr;

            size = 0;
            capacity = 0;

            return false;
        }

        void dtor() {
            if (isCompact()) {
                // Compact buf is intended for use on built-in types without dtors, I guess

                for (unsigned i = 0; i < size; ++i) {
                    //compactBuf[i].dtor();
                    compactBuf[i] = {};
                }
            } else {
                for (unsigned i = 0; i < size; ++i) {
                    //buf[i].dtor();
                    buf[i] = {};
                }
            }

            free(buf);
            buf = nullptr;

            size = 0;
            capacity = 0;
        }

        T &operator[](unsigned ind) {
            assert(size);
            ind = (ind + size) % size;

            assert(ind < size);

            if (isCompact()) {
                return compactBuf[ind];
            }

            assert(buf);

            return buf[ind];
        }

        const T &operator[](unsigned ind) const {
            assert(size);
            ind = (ind + size) % size;

            assert(ind < size);

            if (isCompact()) {
                return compactBuf[ind];
            }

            assert(buf);

            return buf[ind];
        }

        bool append(const T &value) {
            if (isCompact()) {
                if (size + 1 <= COMPACT) {
                    compactBuf[size++] = value;

                    return false;
                }

                capacity = COMPACT + 1;
                T *tmpBuf = (T *)calloc(capacity, sizeof(T));
                TRY_B(!tmpBuf);

                for (unsigned i = 0; i < COMPACT; ++i) {
                    tmpBuf[i] = compactBuf[i];
                    compactBuf[i] = {};
                }

                buf = tmpBuf;
            }

            if (size >= capacity) {
                TRY_B(resize(capacity * 2));

                assert(capacity > size);
            }

            buf[size++] = value;

            return false;
        }

        const T &pop() {
            assert(!isEmpty());

            if (isCompact())
                return compactBuf[--size];

            assert(buf);

            return buf[--size];
        }

        bool isEmpty() const {
            return size == 0;
        }

        unsigned getSize() const {
            return size;
        }

        unsigned getCapacity() const {
            return isCompact() ? COMPACT : capacity;
        }

        constexpr bool isCompact() const {
            return capacity == 0;
        }

    private:

        union {
            T *buf;

            T compactBuf[COMPACT];
        };

        unsigned size;
        unsigned capacity;

        bool resize(unsigned new_capacity) {
            assert(capacity != 0);
            assert(new_capacity >= capacity);

            T *newBuf = (T *)realloc(buf, new_capacity * sizeof(T));
            TRY_B(!newBuf);

            buf = newBuf;
            capacity = new_capacity;

            return false;
        }

    };

}


#endif // VECTOR_H
