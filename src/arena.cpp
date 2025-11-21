#include "arena.hpp"

#include "cstdlib"

Arena::Arena(int buffer_size)
  : m_buffer {static_cast<unsigned char*>(std::malloc(buffer_size))}
  , m_offset {0}
  , m_size {buffer_size}
{
}

void Arena::reset() { m_offset = 0; }

Arena::~Arena()
{
    std::free(m_buffer);
}

