#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#ifndef MINISQL_FILESYSTEM_H
#define MINISQL_FILESYSTEM_H

int directoryExists(const char* path);
int createDirectory(const char* path);
int fileExists(char* filename);
int writeInFile(char *fileName, char* text);
int createFile(char *fileName);
int appendInFile(char *fileName, char* text);
int putInFile(char *fileName, char* text, int addNewLine);
#endif //MINISQL_FILESYSTEM_H
