#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "const.h"
#include "lexer.h"
#include "io.h"
#include "filesystem.h"
#include "database.h"
#include "stdbool.h"
#define MAX_LENGTH 32

void printIntroText(){
    // Color green start
    printf("\033[0;32m");
    printf("MiniSql - v0.1\n");
    printf("=====================================================\n");
    printf("m    m   iii   nnnn    iii   ssss    q q q       ll\n");
    printf("mm  mm    i    n   n    i    ss      q   q    q  ll   \n");
    printf("m mm m    i    n   n    i     sss    qqqqq   q   ll   \n");
    printf("m    m    i    n   n    i        ss      q q     ll   \n");
    printf("m    m   iii   n   n   iii    ssss       qq      lllll\n");
    printf("=====================================================\n\n");
    // Color green end
    printf("\033[0m");
}

// User struct for username and password also confirm the password
struct {
    char username[MAX_LENGTH];
    char password[MAX_LENGTH];
    char confirmPassword[MAX_LENGTH];
} typedef User;



/** It takes an integer argument to determine if password confirmation is needed.
 *  Gets user authentication info from stdin. Verifies if the username is valid
 *  If confirmPassword flag is up then checks for password and Confirm Password inputs
 *  Recursively asks for input.
 *  @param confirmPassword If confirming password is necessary
 *  @returns validated User information
 * */
 User getUserInfo(int confirmPassword){
    // Create a User struct instance to store the user's details.
    fflush(stdin);
    User user;

    // Declare a variable to track the validity of the input.
    bool isValidInput;

    // Loop indefinitely until valid input is received and processed.
    while (1) {
        // Assume input is valid at the beginning of each loop iteration.
        isValidInput = true;
        // Prompt the user for a username and read the input.
        printf("Enter username (max 32 characters): ");
        fgets(user.username, sizeof(user.username), stdin);
        // Check if the newline character is present in the input buffer.
        if (!strchr(user.username, '\n')) {
            // Clear the input buffer if the newline character is missing.
            clearInputBuffer();
            isValidInput = false;
        }
        // Remove the newline character from the username.
        user.username[strcspn(user.username, "\n")] = 0;
        // Check if username length is valid and if the input is valid.
        if (strlen(user.username) > MAX_LENGTH || !isValidInput) {
            printf("Username cannot exceed 32 characters.\n");
            continue;
        }
        // Prompt for password and read the input.
        printf("Enter password (max 32 characters): ");
        fgets(user.password, sizeof(user.password), stdin);
        // Remove the newline character from the password.
        user.password[strcspn(user.password, "\n")] = 0;
        // Check if password length is valid.
        if (strlen(user.password) > MAX_LENGTH) {
            printf("Password cannot exceed 32 characters.\n");
            continue;
        }
        // Check if password confirmation is needed.
        if(confirmPassword == 1){
            // Prompt for password confirmation and read the input.
            printf("Confirm password: ");
            fgets(user.confirmPassword, sizeof(user.confirmPassword), stdin);
            // Remove the newline character from the confirmed password.
            user.confirmPassword[strcspn(user.confirmPassword, "\n")] = 0;
            // Check if the password and confirmed password match.
            if (strcmp(user.password, user.confirmPassword) != 0) {
                printf("Passwords do not match. Try again.\n");
                continue;
            }
        }
        // Break out of the loop if all inputs are valid.
        break;
    }
    // Return the user structure with the collected information.
    return user;
}

int createUser(NodeList *tables){
    printf("Create your account\n");
    User user = getUserInfo(1);
    char *buffer = createBuffer();
    insertInBuffer(&buffer, "INSERT INTO user (username, password) VALUES ('%s', '%s');", user.username, user.password);
    DBOp dbOp = execSQL(buffer, tables);
    clearBuffer(&buffer);
    printDbOp(&dbOp);
    return 1;
}


void initDataDirectory(){
    if (directoryExists(DATA_DIR) != 1) {
        if (createDirectory(DATA_DIR)) {
            printf("Setting up 'minisql database.\n");
        } else {
            printf("Failed to setup database, error: Failed to create data directory .\n");
        }
    }
    char *conf = getTableConfFileName();

    if(fileExists(conf) == 0){
        createFile(conf);
    }
    clearBuffer(&conf);
}

int initialize(NodeList *tables){
    int exists = doesTableExist(tables, "user");
    if(exists == 0){
        execSQL("CREATE TABLE user (id integer primary key, username varchar unique, password varchar, created datetime default now)", tables);
        *tables = loadTables();
        return 0;
    }
    return 1;

}


int authenticate(User user, NodeList *tables){
    char *buffer = createBuffer();
    insertInBuffer(&buffer, "SELECT username, password FROM user where username = '%s';", user.username);
    DBOp dbOp = execSQL(buffer, tables);
    clearBuffer(&buffer);
    if(dbOp.rowCount != 1){
        clearBuffer(&buffer);
        return -1;
    }
    else{
        char* pass = getRowValue(dbOp.rows, 0, 2, dbOp.rowCount);
        if(strcmp(pass, user.password) == 0){
            return 1;
        }
    }
    return 0;
}



int main() {
    printIntroText();
    initDataDirectory();
    NodeList tables = loadTables();
    int setup = initialize(&tables);
    if (setup == 0) {
        createUser(&tables);
    }
    while (1) {
        printf("Login to your account\n");
        User user = getUserInfo(0);
        int auth = authenticate(user, &tables);
        if (auth == 1) {
            printSuccess("Logged in successfully");
            break;
        } else if (auth == -1) {
            printError("User doesn't exist");
        } else {
            printError("User password doesn't match");
        }
    }
    while (1) {
        printf("\n$>> ");
        char *input = handleInput();
        if (input != NULL) {
            if (caseInsensitiveCompare(input, "quit;") == 0) {
                exit(0);
            } else if (caseInsensitiveCompare(input, "create user;") == 0) {
                createUser(&tables);
            } else if (caseInsensitiveCompare(input, "list tables;") == 0) {
                printTables(tables);
                tables = loadTables();
            } else {
                DBOp dbOp = execSQL(input, &tables);
                if (dbOp.code == SUCCESS) {
                    printSuccess("%s", dbOp.successMsg);
                    if(isSelectKeyword(dbOp.action) || isInsertKeyword(dbOp.action)){
                        printDbOp(&dbOp);
                    }

                } else {
                    printError("%s", dbOp.error);
                }
                clearDBOp(&dbOp);
            }
            clearBuffer(&input);
            fflush(stdin);
            printf(" ");
        }

    }
}

