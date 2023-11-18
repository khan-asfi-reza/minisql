#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#ifndef MINISQL_FILESYSTEM_H
#define MINISQL_FILESYSTEM_H

int directory_exists(const char* path);
int create_directory(const char* path);
int fileExists(char* filename);

#endif //MINISQL_FILESYSTEM_H
