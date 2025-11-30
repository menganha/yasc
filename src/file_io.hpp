//
// Routines for file reading and writing
// NOTE: Not yet portable. Only POSIX compatible
//

#pragma once

#include "arena.hpp"

// Reads whole text file into a buffer on the arena
char* fileRead(const char* file_path, Arena& arena);

// Writes "count" chars of buffer into the path as a text file
int fileWrite(char* file_path, char* buffer, int count);

// String manipulation functions
inline int strIsSpace(char const* str);
char*      strFindFirstNonEmpty(const char* str);
char*      strFindCharOrCommentChar(const char* str, char c, char comment_char);
char*      strStripWhitespaceRight(const char* str, char* str_end);
bool       strCompare(const char* str_1, const char* str_2);
