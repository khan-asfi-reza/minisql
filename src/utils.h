#include <ctype.h>
#include "const.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stddef.h>

#ifndef MINISQL_UTILS_H
#define MINISQL_UTILS_H


void removeSingleQuotes(char *str);
int caseInsensitiveCompare(const char *str1, const char *str2) ;

int isKeyword(const char* str) ;
int isDataType(const char* str) ;
int isBuiltInFunc(const char* str) ;
int isPreTableSelectorKeyword(const char* str);
int isNumber(const char *str) ;

int isFilterKeyword(const char* str);
int isLogicalOperator(const char* str);
int isSpecialPunct(char c);

void stringToLower(char *str);
int isSelectKeyword(const char *str);

int isInsertKeyword(const char* str);
int isCreateKeyword(const char* str);
size_t strToLongInt(const char *str);
int isUpdateKeyword(const char* str);
int isDeleteKeyword(const char* str);
int isSymbol(const char *str);
char* replaceString(char *str, size_t idx, size_t endIdx, const char *subString);
int isValueFunc(const char* str);
char* concatStrings(const char *strings[], int count);
void freeMultiple(int count, ...);

void printSuccess(const char *format, ...);
void printError(const char *format, ...);

char *createBuffer();
void insertInBuffer(char **buffer, const char *format, ...);
char *createBufferWithSize(size_t size);
size_t getLine(char **restrict line, size_t *restrict size, FILE *restrict file);
void clearInputBuffer();
char* escapeCommas(const char* input);
size_t max(size_t a, size_t b);
size_t isEven(size_t a);
void clearBuffer(char **buffer);
#endif
//MINISQL_UTILS_H
