#ifndef PJSON_H
#define PJSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//数据结构

typedef struct pjson {
    //原始json字符串
    char * data;
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