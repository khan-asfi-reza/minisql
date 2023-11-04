cmake_minimum_required(VERSION 3.26)
project(minisql C)

set(CMAKE_C_STANDARD 23)

add_executable(minisql src/main.c
        src/lexer.c
        src/const.c
        src/database.c
        src/filesystem.c
        src/io.c
        src/utils.c)
