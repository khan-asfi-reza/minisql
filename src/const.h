#ifndef MINISQL_CONST_H
#define MINISQL_CONST_H

#include <ctype.h>
extern const char *const KEYWORDS[];
extern const size_t LEN_KEYWORDS;
extern const size_t LEN_DATA_TYPES;
extern const char *const DATA_TYPES[];
extern const char *const BUILT_IN_FUNC[];
extern const size_t LEN_BUILT_IN_FUNC;
extern const char *SYNTAX_ERROR_START;
extern const char *const PRE_TB_SELECTOR_KEYWORDS[];
extern const char* const DATE_TYPES[];
extern const size_t LEN_PRE_TB_SELECTOR_KEYWORDS ;
extern const char *INITIAL_FILE;
extern const char *const VALUE_FUNC[];
extern const size_t VALUE_FUNC_LEN;
extern const char *FILTER_KEYWORD;
extern const char *SELECT_KEYWORD;
extern const char *DATA_DIR;


extern const char* const LOGICAL_OP[];

extern const size_t LEN_LOGICAL_OP;

#endif