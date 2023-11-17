#include <ctype.h>
#include "const.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stddef.h>

/**
 * Removes single quote `'` from a string pointer
 *
 * @param str String pointer
 * @return None
 *
 */
void removeSingleQuotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '\'' && str[len - 1] == '\'') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

/**
 * Performs strcmp but case insensitive , 'a', 'A' will return 0
 *
 * @param str1 Comparable string
 * @param str2 Comparable string
 * @return Integer value 0 or any integer,
 * 0 = the the characters are same
 *
 */
int caseInsensitiveCompare(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (toupper(*str1) != toupper(*str2)) {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}


/**
 * Checks if a string is in CONST `DATA_TYPES`
 * Checks if a string is an SQL data type, data types: varchar, text, number, integer, boolean etc
 * @param str Comparable string
 * @return whether or not the string is in the array of DATA_TYPES;
 * @example isDataType("INTEGER") will return 1, isDataType("X") will return 0
 */
int isDataType(const char* str) {
    for (int i = 0; i < LEN_DATA_TYPES; i++) {
        if (caseInsensitiveCompare(str, DATA_TYPES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Converts a string number to a unsigned long long int number
 * String "12" will be converted to -> 12 (decimal)
 * @param str String number
 * @return Decimal number extracted from the string number
 * long long = size_t
 */
size_t strToLongInt(const char *str) {
    char *strPtr;
    size_t value = strtol(str, &strPtr, 10);
    return value;
}


/**
 * Returns 0 or 1 based on string is in CONST `BUILT_IN_FUNC` or not
 * Checks if a string is an sql built in function
 * @param str Comparable string
 * @return whether or not the string is in the array of BUILT_IN_FUNC
 *
 */
int isBuiltInFunc(const char* str) {
    for (int i = 0; i < LEN_BUILT_IN_FUNC; i++) {
        if (caseInsensitiveCompare(str, BUILT_IN_FUNC[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/**
 * Returns 0 or 1 based on string is in CONST `VALUE_FUNC` or not
 * Checks if a string is a default value generator function for sql
 * @param str Comparable string
 * @return whether or not the string is in the array of VALUE_FUNC;
 *
 */
int isValueFunc(const char* str) {
    for (int i = 0; i < VALUE_FUNC_LEN; i++) {
        if (caseInsensitiveCompare(str, VALUE_FUNC[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/**
 * Returns 0 or 1 based on string is in CONST `KEYWORDS` or not
 * Checks if a string is an sql keyword or not
 * @param str Comparable string
 * @return whether or not the string is in the array of KEYWORD
 *
 */
int isKeyword(const char* str) {
    for (int i = 0; i < LEN_KEYWORDS; i++) {
        if (caseInsensitiveCompare(str, KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/**
 * Returns 0 or 1 based on string is in CONST `PRE_TB_SELECTOR_KEYWORDS` or not
 * A pre table selector checks if the argument string is an sql table selector keyword,
 * Example: `SELECT * FROM TABLE_1` here `FROM` is a table selector, by this FROM keyword
 * it can be determined that any identifier next to a table selector points to a database table
 * @param str Comparable string
 * @return whether or not the string is in the array of PRE_TB_SELECTOR_KEYWORDS
 *
 */
int isPreTableSelectorKeyword(const char* str) {
    for (int i = 0; i < LEN_PRE_TB_SELECTOR_KEYWORDS; i++) {
        if (caseInsensitiveCompare(str, PRE_TB_SELECTOR_KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/**
 * Returns 0 or 1 based on string is in CONST `LOGICAL_OP` or not
 * Logical connection between two filter statement, AND and OR
 * Note: This functionality not yet implemented
 * @param str Comparable string
 * @return whether or not the string is in the array of LOGICAL_OP
 *
 */
int isLogicalOperator(const char* str) {
    for (size_t i = 0; i < LEN_LOGICAL_OP; i++) {
        if (caseInsensitiveCompare(str, LOGICAL_OP[i]) == 0) {
            return 1;
        }
    }
    return 0;
}



/**
 * Concat array of string to a new string
 * @param strings[] Array of strings
 * @param count Number of string in the strings array
 * @return New concatenated string
 *
 */
char* concatStrings(const char *strings[], int count) {

    size_t totalLength = 1;
    for (int i = 0; i < count; i++) {
        totalLength += strlen(strings[i]);
    }

    char *result = malloc(totalLength * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(result, strings[i]);
    }

    return result;
}

/**
 * Checks if the string is a number
 * @param str String number
 * @return 1 if the string is number and 0 if the string is not number
 *
 */
int isNumber(const char *str) {

    if (*str == '-' || *str == '+') str++;
    int decimalFound = 0;
    while (*str) {
        if (*str == '.') {
            if (decimalFound) {
                return 0;
            }
            decimalFound = 1;
            str++;
            continue;
        }
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/**
 * Checks if a character is punctuation
 * Except for '\''
 * @param c Comparable character
 * @return 1 if the char is punctuation and 0 if not punctuation
 *
 */
int isSpecialPunct(char c){
    return c != '\'' && ispunct(c) && c != '_';
}

/**
 * Checks if a string is a symbol
 * @param str Comparable string
 * @return 1 if the str is symbol and 0 if not symbol
 *
 */
int isSymbol(const char *str){
    if(strcmp(str, ">=") == 0 || strcmp(str, "<=") == 0 || strcmp(str, "!=") == 0){
        return 1;
    }
    return isSpecialPunct(str[0]);
}

/**
 * Replace string with given start index and end index and replace the string in that block with a new substring
 * @param str Base string
 * @param idx Start index of the substring
 * @param endIdx End index of the substring
 * @param subString Replacing sub-string
 * @return None
 *
 */
char *replaceString(char *str, size_t idx, size_t endIdx, const char *subString) {
    size_t len = strlen(str);
    size_t newLen = strlen(subString);
    size_t ogSubLen = endIdx - idx + 1;
    size_t newTotalLen = len - ogSubLen + newLen;
    char *toCreate = malloc(newTotalLen + 1);
    strcpy(toCreate, str);
    if (idx >= len) {
        return toCreate;
    }
    memmove(toCreate + idx + newLen, toCreate + idx + ogSubLen, len - idx - ogSubLen + 1);
    memcpy(toCreate + idx, subString, newLen);
    return toCreate;
}

/**
 * Lowers every character in a string
 * @param str Base string
 * @return None
 *
 */
void stringToLower(char* str){
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = (char) tolower(str[i]);
    }
}

/**
 * If the string is "SELECT" keyword
 * @param str Base string
 * @return None
 *
 */
int isSelectKeyword(const char* str){
    return caseInsensitiveCompare(str, "SELECT") == 0;
}

/**
 * If the string is "INSERT" keyword
 * @param str Base string
 * @return None
 *
 */
int isInsertKeyword(const char* str){
    return caseInsensitiveCompare(str, "INSERT") == 0;
}


/**
 * If the string is "CREATE" keyword
 * @param str Base string
 * @return None
 *
 */
int isCreateKeyword(const char* str){
    return caseInsensitiveCompare(str, "CREATE") == 0;
}



/**
 * If the string is "WHERE" keyword
 * @param str Base string
 * @return None
 *
 */
int isFilterKeyword(const char* str){
    return caseInsensitiveCompare(str, "WHERE") == 0;
}


/**
 * If the string is "UPDATE" keyword
 * @param str Base string
 * @return None
 *
 */
int isUpdateKeyword(const char* str){
    return caseInsensitiveCompare(str, "UPDATE") == 0;
}


/**
 * If the string is "DELETE" keyword
 * @param str Base string
 * @return None
 *
 */
int isDeleteKeyword(const char* str){
    return caseInsensitiveCompare(str, "DELETE") == 0;
}


/**
 * Print error in red text
 * @param str format, string format
 * @param ... Arguments
 * @return None
 *
 */
void printError(const char *format, ...){
    va_list args;
    va_start(args, format);
    printf("\033[1;31m");
    vprintf(format, args);
    printf("\033[0m\n");
    va_end(args);
}

/**
 * Print success in green text
 * @param str format, string format
 * @param ... Arguments
 * @return None
 *
 */
void printSuccess(const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("\033[32m");
    vprintf(format, args);
    printf("\033[0m\n");
    va_end(args);
}


/**
 * Creates a string buffer with size 1 byte
 * @return buffer string
 */
char *createBuffer() {
    char *buffer = malloc(1);
    if (buffer == NULL) {
        perror("Memory reallocation failed for string buffer");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0';
    return buffer;
}

/**
 * Creates a string buffer with size `size` byte
 * @param size Buffer size in bytes
 * @return buffer string
 */
char *createBufferWithSize(size_t size){
    char *buffer = malloc(size + 1);
    if (buffer == NULL) {
        perror("Memory reallocation failed for string buffer");
        exit(EXIT_FAILURE);
    }
    buffer[size + 1] = '\0';
    return buffer;
}

/**
 * Insert string in a buffer
 * @param buffer Buffer pointer
 * @param format String format
 * @param ... Argument variables
 * @return None
 */
void insertInBuffer(char **buffer, const char *format, ...) {
    // Function divided in two part
    // First part identifies the required size for the new string
    // Second part inserts the string

    va_list args;
    va_start(args, format);
    // Instead of inserting the arg in a string buffer, it is not inserting but rather
    // identifying the number of bytes needed to insert , + 1 for null terminator
    int neededSize = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    if (neededSize <= 0) {
        fprintf(stderr, "Formatting error\n");
        return;
    }
    char *newBuffer = realloc(*buffer, strlen(*buffer) + neededSize);
    if (newBuffer == NULL) {
        perror("Memory reallocation failed for string buffer");
        exit(EXIT_FAILURE);
    }
    *buffer = newBuffer;
    va_start(args, format);
    vsprintf(*buffer + strlen(*buffer), format, args);
    va_end(args);
}

/**
* Frees multiple heap allocated variable
* @param count Number of variables
* @return None
*/
void freeMultiple(int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; ++i) {
        void *ptr = va_arg(args, void *);
        free(ptr);
    }
    va_end(args);
}

/**
 * Clears a buffer
 * @param buffer Buffer pointer
 * @return None
 */
void clearBuffer(char **buffer) {
    if(buffer != NULL){
        if(*buffer != NULL){
            free(*buffer);
        }
        *buffer = NULL;
    }
}

/**
 * Maximum value between two number
 * @param a
 * @param b
 * @return max value
 */
size_t max(size_t a, size_t b){
    return a > b ? a : b;
}

/**
 * If a number is even or odd
 * @param a
 * @return True of False
 */
size_t isEven(size_t a){
    return (a & 1) == 0;
}

/**
 * If a sql string type, value has ',' inside then escape the commas with '\\,'
 * @param input String input
 * @return Escaped comma string
 */
char* escapeCommas(const char* input) {
    int cCount = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ',') {
            cCount++;
        }
    }
    char* result = malloc(strlen(input) + cCount + 1);
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ',') {
            result[j++] = '\\';
        }
        result[j++] = input[i];
    }
    result[j] = '\0';
    return result;
}

/**
 * Reads each line from a file
 * @param line Line pointer
 * @param n Line size
 * @param file File pointer
 * @return If the line was successfully captured
 */
size_t getLine(char **line, size_t *n, FILE *file) {
    if (line == NULL || n == NULL || file == NULL) {
        return -1;
    }
    if (*line == NULL) {
        *n = 128;
        *line = malloc(*n);
        if (*line == NULL) {
            return -1;
        }
    }
    size_t i = 0;
    while (1) {
        int c = fgetc(file);
        if (c == EOF) {
            if (i == 0) {
                return -1;
            }
            break;
        }
        if (i >= *n - 1) {
            size_t new_size = *n * 2;
            char *new_ptr = realloc(*line, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *line = new_ptr;
            *n = new_size;
        }
        (*line)[i++] = (char)c;
        if (c == '\n') {
            break;
        }
    }
    (*line)[i] = '\0';
    return i;
}


/**
 *T his function is often used after reading input using functions like
 * `scan_f` or `f_gets`.
 * These functions may leave a newline character or other unwanted characters in the input buffer.
 * If not cleared, these leftover characters can be mistakenly read by subsequent input operations,
 * leading to logical errors in the program. By calling clearInputBuffer,
 * ensure that the input buffer is clean before the next input operation.
 */
void clearInputBuffer() {
    int c;
    /**
     * This is a while loop that continuously calls getchar()
     * to read the next character from the standard input.
     * It assigns the character to c and then checks two conditions:
     *
     * - If c is not equal to the newline character ('\n'),
     *   which usually signifies the end of a line of input.
     * - If c is not the special EOF value,
     *   which signifies the end of the input stream (for example, when the user has no more input to provide,
     *   or if you're reading from a file and reach its end).
     *  The loop continues until either a newline character is found or the end of the file/input stream is reached.
     *  During each iteration, the function effectively discards the character read.
     */
    while ((c = getchar()) != '\n' && c != EOF) { }
}