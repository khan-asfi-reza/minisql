#include <stddef.h>

/*
 * SQL Keywords, Anything except this is an Identifier
 */
const char *const KEYWORDS[] = {
        "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE",
        "FROM", "WHERE", "SET", "VALUES", "INTO", "TABLE",
        "LIMIT", "OFFSET",
        "AND", "OR", "AS"
};

/*
 * SQL Keywords and their concurrent keyword is a table selector, Example:
 * SELECT * FROM user; here user resides after from, therefore FROM is a pre table selector keyword
 */
const char *const PRE_TB_SELECTOR_KEYWORDS[] = {
        "UPDATE", "DELETE",
        "FROM", "INTO", "TABLE",
};

const size_t  LEN_PRE_TB_SELECTOR_KEYWORDS = sizeof(PRE_TB_SELECTOR_KEYWORDS) / sizeof(PRE_TB_SELECTOR_KEYWORDS[0]);


const size_t LEN_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

/*
 * SQL Data Types
 */
const char *const DATA_TYPES[] = {
        "INTEGER", "FLOAT", "TEXT",
        "VARCHAR", "BOOLEAN", "DATETIME",
        "DATE", "TIME", "SERIAL", "INT"
};

/*
 * SQL Logical Operator
 */
const char* const LOGICAL_OP[] = {
        "AND", "OR",
};

const size_t LEN_LOGICAL_OP = 2;

const size_t LEN_DATA_TYPES = sizeof(DATA_TYPES) / sizeof(DATA_TYPES[0]);

/*
 * Built-In Functions with value
 */
const char *const BUILT_IN_FUNC[] = {
        "UNIQUE", "NOW", "RANDOM", "UUID", "NULL", "PRIMARY", "KEY", "FOREIGN", "NOT", "DEFAULT"
};

/*
 * Built In Value generating function
 */
const char *const VALUE_FUNC[] = {
        "NOW", "RANDOM", "UUID", "NULL",
};

const size_t VALUE_FUNC_LEN = 4;

/*
 * Different Date format
 */
const char* const DATE_TYPES[] = {
        "DATE", "TIME", "DATETIME"
};

const char* DATA_DIR = "data";

const size_t LEN_BUILT_IN_FUNC = sizeof(BUILT_IN_FUNC) / sizeof(BUILT_IN_FUNC[0]);

const char *SYNTAX_ERROR_START = "SQL Syntax Error: `";