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

        static const unsigned DEFAULT_CAPACITY = 8;

        FACTORIES(Vector);

        bool ctor() {
            buf = (T *)calloc(DEFAULT_CAPACITY, sizeof(T));
            if (!buf)
                return true;

            size = 0;
            capacity = DEFAULT_CAPACITY;

            return false;
        }

        void dtor() {
            free(buf);
            buf = nullptr;

            size = 0;
            capacity = 0;
        }

        T &operator[](size_t ind) {
            assert(ind < size);
            assert(buf);

            return buf[ind];
        }

        const T &operator[](size_t ind) const {
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

        T &pop() {
            assert(buf);
            assert(!isEmpty());

            return buf[--size];
        }

        bool isEmpty() const {
            return size == 0;
        }

        size_t getSize() const {
            return size;
        }

        size_t getCapacity() const {
            return capacity;
        }

    private:

        T *buf;

        size_t size;
        size_t capacity;

        bool resize(unsigned new_capacity) {
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
