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
#include <time.h>
#define MAX_LENGTH 32

void printIntroText(){
    printf("\033[0;32m");
    printf("Booting MiniSql - v1\n");
    printf("=====================================================\n");
    printf("m    m   iii   nnnn    iii   ssss    q q q       ll\n");
    printf("mm  mm    i    n   n    i    ss      q   q    q  ll   \n");
    printf("m mm m    i    n   n    i     sss    qqqqq   q   ll   \n");
    printf("m    m    i    n   n    i        ss      q q     ll   \n");
    printf("m    m   iii   n   n   iii    ssss       qq      lllll\n");
    printf("=====================================================\n\n");
    printf("\033[0m");
}

struct {
    char username[MAX_LENGTH];
    char password[MAX_LENGTH];
    char confirmPassword[MAX_LENGTH];
} typedef User;



void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}


User getUserInfo(){
    fflush(stdin);
    User user;
    bool isValidInput;
    printf("Create your account\n");

    while (1) {
        isValidInput = true;

        printf("Enter username (max 32 characters): ");
        fgets(user.username, sizeof(user.username), stdin);
        if (!strchr(user.username, '\n')) {
            clearInputBuffer();
            isValidInput = false;
        }
        user.username[strcspn(user.username, "\n")] = 0;

        if (strlen(user.username) > MAX_LENGTH || !isValidInput) {
            printf("Username cannot exceed 32 characters.\n");
            continue;
        }

        printf("Enter password (max 32 characters): ");
        fgets(user.password, sizeof(user.password), stdin);
        user.password[strcspn(user.password, "\n")] = 0;

        if (strlen(user.password) > MAX_LENGTH) {
            printf("Password cannot exceed 32 characters.\n");
            continue;
        }
        printf("Confirm password: ");
        fgets(user.confirmPassword, sizeof(user.confirmPassword), stdin);
        user.confirmPassword[strcspn(user.confirmPassword, "\n")] = 0;

        if (strcmp(user.password, user.confirmPassword) != 0) {
            printf("Passwords do not match. Try again.\n");
            continue;
        }

        break;
    }
    return user;

}

int createUser(Node refNode){
    User user = getUserInfo();
    int requiredSize = snprintf(NULL, 0, "INSERT INTO TABLE user (username, password) VALUES ('%s', '%s')", user.username, user.password) + 1; // +1 for null terminator
    char *query = malloc(requiredSize);
    if (query == NULL) {
        perror("Malloc failed");
        return -1;
    }
    snprintf(query, requiredSize, "INSERT INTO user (username, password) VALUES ('%s', '%s')", user.username, user.password);
    TokenRet tokenRet= lexAnalyze(
            query
    );
    Node node__ = createASTNode(tokenRet);
    DBOp dbOp = dbInsert(node__, refNode);
    printDbOp(&dbOp);
    return 1;
}


void initialize(){
    if (directory_exists(DATA_DIR) != 1) {
        if (create_directory(DATA_DIR)) {
            printf("Setting up 'minisql database.\n");
        } else {
            printf("Failed to setup database, error: Failed to create data directory .\n");
        }
    }
    const char* tableSqlList[] = {DATA_DIR, "/table_user_sql"};
    const char* tableNameList[] = {DATA_DIR, "/table_user"};

    char* tableSql = concatStrings(tableSqlList, 2);
    char* tableName = concatStrings(tableNameList, 2);

    if(fileExists(tableSql) == 0 || fileExists(tableName) == 0){
        TokenRet tokenRet= lexAnalyze(
                "CREATE TABLE user (id integer primary key, username varchar unique, password varchar, created datetime default now)"
        );
        Node node = createASTNode(tokenRet);
        dbCreateTable(node);
        createUser(node);
    }

}


int main() {

    printIntroText();
    initialize();
    NodeList tableList = loadTables();

    while (1) {
        printf("\n$>> ");
        // Takes input
        char *input = handleInput();
        if(input != NULL){
            if(caseInsensitiveCompare(input, "quit;") == 0){
                exit(0);
            }
            else if(caseInsensitiveCompare(input, "create user;") == 0){
                Node *userTable = getNodeFromList(&tableList, "user");
                createUser(*userTable);
            }
            else{
                TokenRet tokenRet= lexAnalyze(input);
                Node node = createASTNode(tokenRet);
                if(node.isInvalid == 0){
                    Node *tableNode = getNodeFromList(&tableList, node.table.value);
                    if(tableNode != NULL){
                        if(isSelectKeyword(node.action.value)){
                            DBOp dbOp = dbSelect(node, *tableNode);
                            if(dbOp.code == SUCCESS){
                                printDbOp(&dbOp);
                            }
                            else{
                                printError("%s", dbOp.error);
                            }
                            clearDBOp(&dbOp);
                        }
                        else if(isInsertKeyword(node.action.value)){
                            DBOp dbOp = dbInsert(node, *tableNode);
                            if(dbOp.code == SUCCESS){
                                printDbOp(&dbOp);
                            }
                            else{
                                printError("%s", dbOp.error);
                            }
                            clearDBOp(&dbOp);
                        }
                        else if(isDeleteKeyword(node.action.value)){
                            DBOp dbOp = dbDelete(node, *tableNode);
                            if(dbOp.code == SUCCESS){
                                printSuccess("%s", dbOp.successMsg);
                            }
                            else{
                                printError("%s", dbOp.error);
                            }
                            clearDBOp(&dbOp);
                        }
                        else if(isUpdateKeyword(node.action.value)){
                            DBOp dbOp = dbUpdate(node, *tableNode);
                            if(dbOp.code == SUCCESS){
                                printSuccess("%s", dbOp.successMsg);
                            }
                            else{
                                printError("%s", dbOp.error);
                            }
                            clearDBOp(&dbOp);
                        }
                        else{
                            printError("Invalid sql command, command not recognized");
                        }
                    }
                    else{
                        if(caseInsensitiveCompare(node.action.value, "CREATE") == 0){
                            DBOp dbOp = dbCreateTable(node);
                            printSuccess("%s", dbOp.successMsg);
                            clearDBOp(&dbOp);
                        }
                        else{
                            printError("Table `%s` doesn't exist", node.table.value);
                        }
                    }

                }
                fflush(stdin);
                printf(" ");
            }
        }

    }

}