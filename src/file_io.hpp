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
