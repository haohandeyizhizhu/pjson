#include "pjson.h"

static int pjson_util_is_space(char c);
static int pjson_util_is_number(char c);
static int pjson_util_is_hex_number(char c);

/**
 * @param p 指向pjson对象的指针
 * @brief 初始化pjson对象
 */
void pjson_init(pjson * p)
{
    if (!p) return;
    p->data = NULL;
    p->token_arr = NULL;
    p->token_capacity = 0;
    p->token_count = 0;
        
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
    free(p->token_arr);
    p->data = NULL;
    p->token_arr = NULL;
    p->token_capacity = 0;
    p->token_count = 0;
}

/**
 * @param p 指向pjson对象的指针
 * @param t 要追加的token
 * @return 存放错误信息的结构体
 * @brief 向pjson的token_arr动态数组追加元素，并实现自动扩容
 */
pjson_err pjson_token_arr_append(pjson * p, token t)
{
    pjson_err ret;

    if (!p){ret.code = 100; ret.msg = "Err: Pointer is NULL"; return ret; }

    if (p->token_arr == NULL && (p->token_capacity || p->token_count)){
        ret.code = 100;
        ret.msg = "Err: Empty arr shouldn't have items";
        return ret;}

    if (p->token_arr != NULL && p->token_capacity == 0){   
        ret.code = 100;
        ret.msg = "Err: Failed to find the capacity of a existed arr.";
        return ret;
    }

    if (p->token_arr == NULL)//空数组
    {
        p->token_arr = malloc(sizeof(token) * PJSON_TOKEN_ARR_INIT_CAP);
        if (!p->token_arr){
            ret.code = 100;
            ret.msg = "Err: Failed to allocate memory";
            return ret;
        }
        p->token_capacity = PJSON_TOKEN_ARR_INIT_CAP;
        p->token_count += 1;
        p->token_arr[0] = t;

        ret.code = 0;
        ret.msg = "No error detected.";
        return ret;
    }
    else //已有元素
    {
        if (p->token_count < p->token_capacity) //没有满
        {
            p->token_arr[p->token_count] = t;
            p->token_count += 1;

            ret.code = 0;
            ret.msg = "No error detected.";
            return ret;
        }
        else if (p->token_count == p->token_capacity) //数组已满
        {
            token * tmp = realloc(p->token_arr, 
                p->token_capacity * PJSON_TOKEN_ARR_GROW_FACTOR * sizeof(token));
            if (!tmp)
            {
                ret.code = 100;
                ret.msg = "Err: Failed to reallocate memory";
                return ret;
            }
            p->token_arr = tmp;
            p->token_capacity *= PJSON_TOKEN_ARR_GROW_FACTOR;
            p->token_arr[p->token_count] = t;
            p->token_count += 1;

            ret.code = 0;
            ret.msg = "No error detected.";
            return ret;
        }
        else //已有元素比总容量大，错误情况
        {
            ret.code = 100;
            ret.msg = "Err: count above capacity.";
            return ret;
        }
    }
}

pjson_err pjson_lex(pjson * p)
{
    pjson_err ret;
    token t;
    const char * s = NULL; //用于在匹配多字符token时，记录起点

    if (!p){ret.code = 100; ret.msg = "Err: Pointer is NULL."; return ret;}
    if (!p->data){ret.code = 100; ret.msg = "Err: string not found."; return ret;}

    const char * cursor = p->data;

    while (1)
    {
        pjson_lex_skip_space(&cursor);
        switch (*cursor)
        {
            case '{':
                t.len = 1; t.start = cursor; t.type = TOKEN_LEFT_BRACE;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                cursor++;
                break;
            case '}':
                t.len = 1; t.start = cursor; t.type = TOKEN_RIGHT_BRACE;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                cursor++;
                break;
            case '[':
                t.len = 1; t.start = cursor; t.type = TOKEN_LEFT_BRACKET;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                cursor++;
                break;
            case ']':
                t.len = 1; t.start = cursor; t.type = TOKEN_RIGHT_BRACKET;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                cursor++;
                break;
            case ',':
                t.len = 1; t.start = cursor; t.type = TOKEN_COMMA;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                cursor++;
                break;
            case ':':
                t.len = 1; t.start = cursor; t.type = TOKEN_COLON;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                cursor++;
                break;
            case 't':
                s = cursor;//存放token开始位置
                ret = pjson_lex_expect(&cursor, "true");
                if (ret.code) return ret;
                t.len = 4; t.start = s; t.type = TOKEN_TRUE;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                break;
            case 'f':
                s = cursor;//存放token开始位置
                ret = pjson_lex_expect(&cursor, "false");
                if (ret.code) return ret;
                t.len = 5; t.start = s; t.type = TOKEN_FALSE;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                break;
            case 'n':
                s = cursor;//存放token开始位置
                ret = pjson_lex_expect(&cursor, "null");
                if (ret.code) return ret;
                t.len = 4; t.start = s; t.type = TOKEN_NULL;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                break;
            case '-': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                s = cursor;
                ret = pjson_lex_number(&cursor);
                if (ret.code) return ret;
                t.len = cursor - s; t.start = s; t.type = TOKEN_NUMBER;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                break;
            case '\0':
                t.len = 0; t.start = cursor; t.type = TOKEN_EOF;
                ret = pjson_token_arr_append(p, t);
                if (ret.code) return ret;
                ret.code = 0; ret.msg = "No error detected.";
                return ret;
            default:
                ret.code = 100; ret.msg = "Err: Unknown token.";
                return ret;
        }
    }


}

/**
 * @param cursor 指向目标字符串指针的指针
 * @return 包含错误信息的结构体
 * @brief 这个函数从从字符串*cursor开始的位置开始读取，期望得到一个合法json数字，如果成功，*cursor最终指向
 * 合法json数字后的第一个字符，并返回错误码0，如果失败，返回错误码100，并“不”保证*cursor光标指向错误位置
 */
pjson_err pjson_lex_number(const char ** cursor)
{
    pjson_err ret;

    if (!cursor){ret.code = 100; ret.msg = "Err: Pointer is NULL"; return ret;}
    if (!(*cursor))
        {ret.code = 100; ret.msg = "Err: Pointer is NULL"; return ret;}
    
    if ((**cursor) == '-') //跳过可能的负号
        (*cursor)++;
    
    /*整数部分判断*/
    if (!pjson_util_is_number(**cursor))
        {ret.code = 100; ret.msg = "Err: Missing integer part"; return ret;}
    if (**cursor == '0')
    {
        (*cursor)++;
        if (pjson_util_is_number(**cursor))
            {ret.code = 100; ret.msg = "Err: Invalid leading zero(s)"; return ret;}
    }
    else
    {
        (*cursor)++;
        while (pjson_util_is_number(**cursor))
            (*cursor)++;
    }

    /*可选小数部分判断*/
    if (**cursor == '.')
    {
        (*cursor)++;
        if (!pjson_util_is_number(**cursor))
            {ret.code = 100; ret.msg = "Err: min 1 digit"; return ret;}
        while (pjson_util_is_number(**cursor))
            (*cursor)++;
    }

    /*可选指数部分判断*/
    if (**cursor == 'e' || **cursor == 'E')
    {
        (*cursor)++;
        if (**cursor == '+' || **cursor == '-')
            (*cursor)++;
        if (!pjson_util_is_number(**cursor))
            {ret.code = 100; ret.msg = "Err: min 1 digit"; return ret;}
        while (pjson_util_is_number(**cursor))
            (*cursor)++;
    }

    ret.code = 0;
    ret.msg = "No error detected.";
    return ret;
}

/**
 * 
 */
pjson_err pjson_lex_string(const char ** cursor)
{
    pjson_err ret;
    int is_excaping = 0; //是否处于转义状态

    if (!cursor){ret.code = 100; ret.msg = "Err: Pointer is NULL"; return ret;}
    if (!(*cursor))
        {ret.code = 100; ret.msg = "Err: Pointer is NULL"; return ret;}
    
    /*确认第一个字符为双引号，并把光标移到其后第一个字符,开始主循环*/
    if ((**cursor) != '"')
        {ret.code = 100; ret.msg = "Err: String not begin with \"."; return ret;}
    (*cursor)++;

    while (1)
    {
        if (is_excaping)
        {
            if ((**cursor) == '"' || (**cursor) == '\\' || (**cursor) == '/' ||
                (**cursor) == 'b' || (**cursor) == 'f' || (**cursor) == 'n' ||
                (**cursor) == 'r' || (**cursor) == 't')
            {
                is_excaping = 0;
                (*cursor)++;
            }
            else if ((**cursor) == 'u')
            {
                int legal = 1;
                for (int i = 1; i <= 4; i++)
                {
                    if (!pjson_util_is_hex_number(*((*cursor) + i)))
                    {
                        legal = 0;
                        break;
                    }
                }

                if (!legal)
                {
                    ret.code = 100; ret.msg = "Err: Invalid escape sequence"; return ret;
                }
                else
                {
                    is_excaping = 0;
                    (*cursor) += 5;
                }
            }
            else
            {
                ret.code = 100; ret.msg = "Err: Invalid escape sequence"; return ret;
            }
        }
        else
        {
            if ((**cursor) == '\0')
            {
                ret.code = 100; ret.msg = "Err: string not closed."; return ret;
            }
            else if ((unsigned char)(**cursor) < 0x20)
            {
                ret.code = 100; ret.msg = "Err: No raw control chars."; return ret;
            }
            else if ((**cursor) == '"')
            {
                (*cursor)++;
                ret.code = 0; ret.msg = "No error detected."; return ret;
            }
            else if ((**cursor) == '\\')
            {
                is_excaping = 1;
                (*cursor)++;
            }
            else
            {
                (*cursor)++;
            }
                
        }
    }
}

/**
 * @param cursor 指向目标字符串指针的指针
 * @param str 预期匹配到的字符串
 * @return 包含错误信息的结构体
 * @brief 这个函数“期望”字符串*cursor从当前位置开始可以得到完整的str字符串，如果匹配成功，返回错误码0并将*cursor光标指向
 * str子串后的下一个字节，如果匹配失败则直接返回错误码100，*cursor光标指向出错的位置
 */
pjson_err pjson_lex_expect(const char ** cursor, const char * str)
{
    pjson_err ret;

    if (!str || !cursor){ret.code = 100; ret.msg = "Err: Pointer is NULL."; return ret;}
    if (!*cursor){ret.code = 100; ret.msg = "Err: Pointer is NULL."; return ret;}

    size_t len = strlen(str);
    size_t i;
    for (i = 0; i < len; i++)
    {
        if ((**cursor) != str[i] || (**cursor) == '\0')
        {
            ret.code = 100;
            ret.msg = "Err: Failed to expect.";
            return ret;
        }
        (*cursor)++;
    }
    ret.code = 0;
    ret.msg = "No error detected.";
    return ret;
}

/**
 * @param cursor 指向目标字符串指针的指针
 * @brief 该函数接受指向待跳过字符串指针的指针，将目标字符串指针持续前移直到指向第一个非json合法空白字符
 */
void pjson_lex_skip_space(const char ** cursor)
{
    while (pjson_util_is_space(**cursor))
    {
        (*cursor)++;
    }
}

/**
 * @param c 要判断的字符
 * @return 是合法空白字符返回1，不是则返回0
 * @brief 判断字符是否为合法的json空白字符
 */
static int pjson_util_is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 * @param c 要判断的字符
 * @return 是数字0-9则返回1，不是则返回0
 * @brief 判断字符是否为数字0-9
 */
static int pjson_util_is_number(char c)
{
    return (c >= '0' && c <= '9');
}

/**
 * @param c 要判断的字符
 * @return 是十六进制数字0-F返回1，不是则返回0
 * @brief 判断字符是否为十六进制数字，大小写均可
 */
static int pjson_util_is_hex_number(char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F'));
}