//
//  Simple bump allocator implementation
//

#pragma once

#include "log.hpp"

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)
#define TERABYTES(value) (GIGABYTES(value)*1024LL)

class Arena
{
    unsigned char* m_buffer;
    int            m_offset;
    int            m_size;

  public:
    Arena(int buffer_size);
    ~Arena();
    void reset();
    template<typename T>
    T* allocate(int number = 1);
};

template<typename T>
T* Arena::allocate(int number)
{
    int alignment = alignof(T);
    int size = sizeof(T) * number;

    LASSERT(((alignment & (alignment - 1)) == 0), "Alignment is not a power of two:  %i", alignment);

    std::ptrdiff_t padding = -m_offset & (alignment - 1);

    m_offset += padding;

    LASSERT(((m_offset + size > m_size) == 0), "Not enough memory to allocate object");

    T* result = reinterpret_cast<T*>(&m_buffer[m_offset]);

    m_offset += size;

    return result;
}
