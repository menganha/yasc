#include "file_io.hpp"

#include "log.hpp"

#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

char* fileRead(const char* file_path, Arena& arena)
{
    std::FILE* file_handle = std::fopen(file_path, "r");
    if ( not file_handle )
    {
        LERROR("Could not open file %s", file_path);
        return nullptr;
    }

    struct stat sb;
    if ( fstat(fileno(file_handle), &sb) != 0 )
    {
        LERROR("'fstat' failed for file '%s': erro code %i", file_path, errno);
        return nullptr;
    }

    char* string_contents = arena.allocate<char>(sb.st_size + 1);
    std::fread(string_contents, sizeof(char), sb.st_size, file_handle);
    std::fclose(file_handle);

    return string_contents;
}

int fileWrite(char* file_path, unsigned char* buffer, int count)
{
    std::FILE* file_handle = std::fopen(file_path, "w");
    if ( not file_handle )
    {
        LERROR("Could not open file %s", file_path);
        return 0;
    }
    std::size_t written = std::fwrite(buffer, sizeof(char), count, file_handle);
    fclose(file_handle);
    return (int)written;
}

inline int strIsSpace(char const* str)
{
    return '\0' < *str && *str <= ' ';
}

char* strFindFirstNonEmpty(char const* str)
{
    while ( strIsSpace(str) )
    {
        str++;
    }
    return (char*)str;
}

char* strStripWhitespaceRight(const char* str, char* str_end)
{
    str_end--;
    while ( str_end > str && strIsSpace(str_end) )
    {
        *str_end = '\0';
        str_end--;
    }
    return (char*)str;
}

bool strCompare(const char* str_1, const char* str_2)
{
    while ( *str_1 && (*str_1 == *str_2) )
    {
        str_1++;
        str_2++;
    }
    return *str_1 == *str_2;
}

char* strFindCharOrCommentChar(char const* str, char schar, char comment_char)
{
    int was_whitespace = 0;
    while ( *str && *str != schar && !(was_whitespace && *str == comment_char) )
    {
        was_whitespace = strIsSpace(str);
        str++;
    }
    return (char*)str;
}
