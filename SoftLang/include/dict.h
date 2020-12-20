#ifndef DICT_H
#define DICT_H

#include "general.h"
#include "lexer.h"

#include <cstdlib>
#include <cstring>
#include <cassert>


namespace SoftLang {

    template <typename T>
    class NameDict {
    public:

        struct Item {
            const Token *key;
            T value;
        };

        static const unsigned DEFAULT_CAPACITY = 16;

        FACTORIES(NameDict);

        bool ctor() {
            buf = (Item *)calloc(DEFAULT_CAPACITY, sizeof(Item));
            if (!buf)
                return true;

            capacity = DEFAULT_CAPACITY;
            size = 0;

            return false;
        }

        void dtor() {
            free(buf);
            buf = nullptr;

            capacity = 0;
            size = 0;
        }

        bool set(const Token *key, const T &value) {
            Item *tmp = find(key);

            if (tmp) {
                tmp->value = value;
                return false;
            }

            if (size + 1 > capacity) {
                TRY_B(resize(capacity * 2));
            }

            buf[size++] = {key, value};

            return false;
        }

        // Returns result, not error code - error is impossible
        bool contains(const Token *key) const {
            return find(key);
        }

        // Same as above - error is impossible; Returns the same as `contains`
        bool get(const Token *key, T *dest) const {
            const Item *tmp = find(key);

            if (!tmp)
                return false;

            if (dest)
                *dest = tmp->value;

            return true;
        }

        const T &get(const Token *key, const T &defaultVal={}) const {
            const Item *tmp = find(key);

            if (tmp)
                return tmp->value;

            return defaultVal;
        }

        void erase(const Token *key) {
            Item *tmp = find(key);

            if (!tmp)  return;

            tmp->key = nullptr;
            tmp->value = {};

            *tmp = buf[--size];
        }

    private:

        Item *buf;

        unsigned size;
        unsigned capacity;

        bool resize(unsigned new_capacity) {
            assert(new_capacity >= capacity);

            Item *newBuf = (Item *)realloc(buf, new_capacity * sizeof(Item));
            TRY_B(!newBuf);

            buf = newBuf;
            capacity = new_capacity;

            return false;
        }

        Item *find(const Token *key) const {
            for (unsigned i = 0; i < size; ++i) {
                if (key->getLength() == buf[i].key->getLength() &&
                    strncmp(key->getStr(), buf[i].key->getStr(), key->getLength()) == 0) {

                    return &buf[i];
                }
            }

            return nullptr;
        }

    };

}


#endif // DICT_H
