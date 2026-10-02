#ifndef PJSON_H
#define PJSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//数据结构

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_STRING,
    TOKEN_NUMBER
} token_type; //token类型枚举

typedef struct token {
    token_type type;
    const char * start;
    size_t len;
} token; //token类型

typedef struct pjson {
    //原始json字符串
    char * data;

    //词法分析相关字段
    token * token_arr; //存放token的动态数组
    size_t token_capacity;  //数组容量
    size_t token_count; //当前数组存放的token数量
} pjson;

typedef struct pjson_err{
    int code;
    const char * msg;
} pjson_err;

//函数原型
void pjson_init(pjson * p);
pjson_err pjson_load_str(pjson * p, const char * str);
pjson_err pjson_load_file(pjson * p, const char * path);
void pjson_free(pjson * p);

#endif