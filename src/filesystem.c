#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

int directory_exists(const char* path) {
#ifdef _WIN32
    DWORD ftyp = GetFileAttributesA(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return 0;

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return 1;

    return 0;
#else
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
#endif
}

int create_directory(const char* path) {
#ifdef _WIN32
    return CreateDirectoryA(path, NULL) || ERROR_ALREADY_EXISTS == GetLastError();
#else
    return mkdir(path, 0777) == 0 || errno == EEXIST;
#endif
}

int fileExists(char* filename){
    FILE *file = NULL;
    file = fopen(filename, "r");
    if(file != NULL){
        fclose(file);
        return 1;
    }
    return 0;
}