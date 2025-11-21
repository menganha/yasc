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
