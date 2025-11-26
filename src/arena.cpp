#include "arena.hpp"

#include "cstdlib"

Arena::Arena(int buffer_size)
  : m_buffer {static_cast<unsigned char*>(std::malloc(buffer_size))}
  , m_offset {0}
  , m_size {buffer_size}
{
}

void Arena::reset() { m_offset = 0; }

void Arena::set_mark()
{
    LASSERT(m_mark == -1, "Mark has been already set at %i, %p", m_mark, m_buffer + m_mark)
    m_mark = m_offset;
}

void Arena::reset_to_mark()
{
    LASSERT(m_mark != -1, "Mark has not been set")
    m_offset = m_mark;
    m_mark = -1;
}

Arena::~Arena()
{
    std::free(m_buffer);
}

