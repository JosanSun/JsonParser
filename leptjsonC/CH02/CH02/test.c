#include "leptjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        ++test_count;\
        if(equality)\
            ++test_pass;\
        else{\
             fprintf(stderr, "%s: %d line -- expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
             main_ret = 1;\
            }\
    }while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_LITERAL(init_type, json)\
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(init_type, lept_get_type(&v));\
    }while(0)

static void test_parse_literal( )
{
    TEST_LITERAL(LEPT_NULL, "null");
    TEST_LITERAL(LEPT_TRUE, "true");
    TEST_LITERAL(LEPT_FALSE, "false");
}

//static void test_parse_null( )
//{
//    lept_value v;
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " null "));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//}
//
//static void test_parse_true( )
//{
//    lept_value v;
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " true "));
//    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
//}
//
//static void test_parse_false( )
//{
//    lept_value v;
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
//    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
//}

#define TEST_NUMBER(expect, json)\
    do {\
        lept_value v;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
    }while(0)

static void test_parse_number( )
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");

    TEST_NUMBER(1e10, "1E10");

    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000");   /* must underflow */

    /* boundary value */
    TEST_NUMBER(5e-324, "5e-324");
    TEST_NUMBER(1.7976E308, "1.7976E308");
    //TEST_NUMBER(12345678901234567890, "12345678901234567090");
    
    /* the smallest number > 1 */
    TEST_NUMBER(1.0000000000000001, "1.0000000000000001");
    /* minimum denormal */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    /* Max subnormal double */
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    /* Min normal positive double */
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    /* Max double */
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
 
}

#define TEST_ERROR(error, json)\
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
    }while(0)

static void test_parse_expect_value( )
{
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value( )
{
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");

#if 1
    /* invalid number */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123");  /* at least one digit before '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");    /* at least one digit after '.'  */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
#endif
}

static void test_parse_root_not_singular( )
{
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
#if 1
    /* invalid number */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123");  /* after zero should be '.' or nothing */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}

//3.1234567890123456
//
//17位有效数字
//其中小数点也占有一位。
//但是不包括指数部分

static void test_parse_number_too_big( )
{
#if 1
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

//static void test_parse_expect_value( )
//{
//    lept_value v;
//
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//}
//
//static void test_parse_invalid_value( )
//{
//    lept_value v;
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, " t r"));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//}
//
//static void test_parse_root_not_singular( )
//{
//    lept_value v;
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "  truex "));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//
//    v.type = LEPT_FALSE;
//    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "  false null x "));
//    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
//}

static void test_parse( )
{
    test_parse_literal( );
    test_parse_number( );                     //
    test_parse_expect_value( );               //
    test_parse_invalid_value( );              //
    test_parse_root_not_singular( );          //
    test_parse_number_too_big( );             //
}

int main( )
{
    test_parse( );
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass*100.0 / test_count);
    return main_ret;
}


