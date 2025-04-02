#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)           \
    do {                                                           \
        test_count++;                                              \
        if (equality)                                              \
            test_pass++;                                           \
        else {                                                     \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", \
                    __FILE__, __LINE__, expect, actual);           \
            main_ret = 1;                                          \
        }                                                          \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

/* 测试解析 null */
static void test_parse_null() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

/* 测试解析 true */
static void test_parse_true() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

/* 测试解析 false */
static void test_parse_false() {
    lept_value v;
    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

/* 测试解析数字 */
#define TEST_NUMBER(expect, json)                   \
    do {                                            \
        lept_value v;                               \
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));      \
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));      \
    } while (0)

static void test_parse_number() {
    /* 基本数字 */
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");

    /* 指数形式 */
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(-123.456e+78, "-123.456e+78");

    /* 下溢测试：结果为 0 但合法 */
    TEST_NUMBER(0.0, "1e-10000");

    /* 新增其他合法测试 */
    TEST_NUMBER(0.123, "0.123");
    TEST_NUMBER(-0.123, "-0.123");
    TEST_NUMBER(123.456, "123.456");
    TEST_NUMBER(1e2, "1e2");  /* 等于 100 */
    TEST_NUMBER(1.0e0, "1.0e0");  /* 指数为0 */
}

/* 测试解析错误：缺失值 */
#define TEST_ERROR(error, json)                     \
    do {                                            \
        lept_value v;                               \
        v.type = LEPT_FALSE;                        \
        EXPECT_EQ_INT(error, lept_parse(&v, json)); \
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v)); \
    } while (0)

static void test_parse_expect_value() {
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

/* 测试解析错误：无效的值 */
static void test_parse_invalid_value() {
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

    /* 数字格式错误 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");   /* 不允许 '+' 开头 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_NUMBER(-1.0, "-1");  /* -1 是合法的 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "-1.");  /* 小数点后必须至少有一位数字 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "-0.");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123");  /* 整数部分缺失 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");    /* 小数点后缺少数字 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "00");    /* 不允许前导 0 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "01");    /* 不允许前导 0 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "-.123");  /* 整数部分缺失 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.e10");   /* 小数点后至少需要一位数字 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.2e");    /* 指数部分数字缺失 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.2e+");   /* 指数部分符号后缺少数字 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "--1");     /* 多个符号 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "-+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1..2");    /* 多个小数点 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.2.3");   /* 多个小数点 */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");     
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
}

/* 测试解析错误：根元素后有多余字符 */
static void test_parse_root_not_singular() {
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
    /* 无效数字后多余字符 */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* “0123” 应被视为格式错误 */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

/* 测试解析错误：数字过大 */
static void test_parse_number_too_big() {
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
