#ifndef CHECKSUM_H_GUARD
#define CHECKSUM_H_GUARD

#include <cstdint>


namespace abel {
    /**
     * A type that holds a CRC32 checksum
     */
    class CRC32 {
    public:
        CRC32();

        template <typename T>
        CRC32(const T *data, size_t cnt);

        template <typename T>
        CRC32(const T *data);

        template <typename T>
        void update(const T *data, size_t cnt);

        template <typename T>
        void update(const T *data);

    private:
        uint32_t value;

        static const uint32_t Table[256];
    };
}


#endif // CHECKSUM_H_GUARD


