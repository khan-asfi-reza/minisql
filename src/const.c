//
// Created by Khan Asfi Reza on 24/10/23.
//

#include <ctype.h>

const char *const KEYWORDS[] = {
        "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE",
        "FROM", "WHERE", "SET", "VALUES", "INTO", "TABLE",
        "LIMIT", "OFFSET",
        "AND", "OR", "AS"
};

const char *const PRE_TB_SELECTOR_KEYWORDS[] = {
        "UPDATE", "DELETE",
        "FROM", "INTO", "TABLE",
};

const size_t  LEN_PRE_TB_SELECTOR_KEYWORDS = sizeof(PRE_TB_SELECTOR_KEYWORDS) / sizeof(PRE_TB_SELECTOR_KEYWORDS[0]);


const size_t LEN_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

const char *const DATA_TYPES[] = {
        "INTEGER", "FLOAT", "TEXT",
        "VARCHAR", "BOOLEAN", "DATETIME",
        "DATE", "TIME", "SERIAL"
};

const char* const LOGICAL_OP[] = {
        "AND", "OR",
};

const size_t LEN_LOGICAL_OP = 2;

const size_t LEN_DATA_TYPES = sizeof(DATA_TYPES) / sizeof(DATA_TYPES[0]);

const char *const BUILT_IN_FUNC[] = {
        "UNIQUE", "NOW", "RANDOM", "UUID", "NULL", "PRIMARY", "KEY", "FOREIGN", "NOT", "DEFAULT"
};

const char *const VALUE_FUNC[] = {
        "NOW", "RANDOM", "UUID", "NULL",
};

const size_t VALUE_FUNC_LEN = 4;

const char* const DATE_TYPES[] = {
        "DATE", "TIME", "DATETIME"
};

const char* FILTER_KEYWORD = "WHERE";
const char* SELECT_KEYWORD = "SELECT";
const char* CREATE_KEYWORD = "CREATE";
const char* DATA_DIR = "data";

const size_t LEN_BUILT_IN_FUNC = sizeof(BUILT_IN_FUNC) / sizeof(BUILT_IN_FUNC[0]);

const char *INITIAL_FILE = "init.conf";

const char *SYNTAX_ERROR_START = "SQL Syntax Error: `";