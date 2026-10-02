#include "pjson.h"

/**
 * @param p 指向pjson对象的指针
 * @brief 初始化pjson对象
 */
void pjson_init(pjson * p)
{
    if (p)
        p->data = NULL;
}

/**
 * @param p 指向pjson对象的指针
 * @param str 原始json字符串
 * @return 包含错误信息的pjson_err结构体
 * @brief 将str字符串的内容加载到pjson对象。
 * 调用前应确保使用pjson_init()进行初始化
 * 注意加载时会动态分配内存，调用该函数后应该调用pjson_free()函数释放内存
 */
pjson_err pjson_load_str(pjson * p, const char * str)
{
    pjson_err ret;

    if (!p || !str)
    {
        ret.code = 5;
        ret.msg = "Err: Pointer is NULL";
        return ret;
    }
    
    free(p->data);
    p->data = malloc(strlen(str) + 1);
    if (!p->data)
    {
        ret.code = 4;
        ret.msg = "Err: Failed to allocate memory";
        return ret;
    }
    memcpy(p->data, str, strlen(str) + 1);

    ret.code = 0;
    ret.msg = "No error detected.";

    return ret;
}

/**
 * @param p 指向pjson对象的指针
 * @param path 文件路径
 * @return 包含错误信息的pjson_err结构体
 * @brief 将path对应的json文件内容作为字符串，加载到pjson对象
 * 调用前应确保使用pjson_init()进行初始化
 * 注意内存是动态分配的，调用后应该使用pjson_free()函数释放内存
 */
pjson_err pjson_load_file(pjson * p, const char * path)
{
    pjson_err ret;
    FILE * fp = NULL;

    if (!p || !path)
    {
        ret.code = 5;
        ret.msg = "Err: Pointer is NULL";
        return ret;
    }

    fp = fopen(path, "rb");
    if (!fp)
    {
        ret.code = 1;
        ret.msg = "Err: Failed to open json file";
        return ret;
    }

    if (fseek(fp, 0, SEEK_END))
    {
        fclose(fp);
        ret.code = 2;
        ret.msg = "Err: Failed to seek in file";
        return ret;
    }

    long size = ftell(fp);
    if (size < 0)
    {
        fclose(fp);
        ret.code = 3;
        ret.msg = "Err: Failed to get file length";
        return ret;
    }

    if (fseek(fp, 0, SEEK_SET))
    {
        fclose(fp);
        ret.code = 2;
        ret.msg = "Err: Failed to seek in file";
        return ret;
    }

    char * data = malloc((size_t)size + 1);
    if (!data)
    {
        fclose(fp);
        ret.code = 4;
        ret.msg = "Err: Failed to allocate memory";
        return ret;
    }

    size_t n = fread(data, 1, (size_t)size, fp);
    data[n] = '\0';
    if (n != (size_t)size)
    {
        fclose(fp);
        free(data);

        ret.code = 6;
        ret.msg = "Err: Failed to read enough data";
        return ret;
    }
    free(p->data);
    p->data = data;
    

    fclose(fp);
    ret.code = 0;
    ret.msg = "No error detected.";
    return ret;
}

/**
 * @param p 指向pjson对象的指针
 * @brief 释放先前在加载阶段动态分配的内存
 */
void pjson_free(pjson * p)
{
    if (!p) return;
    free(p->data);
    p->data = NULL;
}