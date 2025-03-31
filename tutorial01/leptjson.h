#ifndef LEPTJSON_H__
#define LEPTJSON_H__

//json类型
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

typedef struct {
    lept_type type;
}lept_value;

//判断结果
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

// 若一个 JSON 只含有空白，传回 LEPT_PARSE_EXPECT_VALUE。
// 若一个值之后，在空白之后还有其他字符，传回 LEPT_PARSE_ROOT_NOT_SINGULAR。
// 若值不是那三种字面值，传回 LEPT_PARSE_INVALID_VALUE。

int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);

#endif /* LEPTJSON_H__ */
