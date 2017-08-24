// _WINDOWS打开内存泄漏检查开关
#define _WINDOWS 1
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "leptjson.h"
#include <stdio.h>
#include <string.h>   //memcmp

//total number of previous test: 289
//total number of current test:  358

//去掉CH05 这个开关

//CH06作为CH05新增的测试用例的开关，
//1表示加入这些新的测试用例，0表示关闭；
//这样很容易比较与之前一章的测试用例不同点
#define CH06 1


//主函数的返回值，0为正常返回，非0存在错误
static int main_ret = 0;
//测试数据的数量
static int test_cnt = 0;
//测试数据的通过数量
static int test_pass = 0;

//------------------CH04所用到的所有宏定义------------
//所有宏定义比较的基本版形式
#define EXPECT_EQ_BASE(equ, expt, actu, format) \
	do {\
		++test_cnt;\
		if(equ)\
			++test_pass;\
		else{\
				fprintf(stderr, "%s: %d line -- expect: " format " actual: " format "\n", __FILE__, __LINE__, expt, actu);\
			main_ret = 1;\
		}\
	}while(0)

//比较两个整数是否相等
#define EXPECT_EQ_INT(expt, actu) EXPECT_EQ_BASE((expt) == (actu), expt, actu, "%d")
//比较两个浮点数是否相等
#define EXPECT_EQ_DOUBLE(expt, actu) EXPECT_EQ_BASE((expt) == (actu), expt, actu, "%.17g")
//比较两个字符串是否相等
#define EXPECT_EQ_STRING(expt, actu, len) \
	EXPECT_EQ_BASE(sizeof(expt) - 1 == len && 0 == memcmp(expt, actu, len), expt, actu, "%s")

//比较两个数组的大小是否相等
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expt, actu) \
		EXPECT_EQ_BASE((expt) == (actu), (size_t)expt, (size_t)actu, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expt, actu) \
		EXPECT_EQ_BASE((expt) == (actu), (size_t)expt, (size_t)actu, "%zu")
#endif

//重构代码，将原来的测试简化 TEST_LITERAL可以用来测试null, true, false
//它们的共同点都是解析成功，即返回LEPT_PARSE_OK;但是解析出来的元素类型存在不同
#define TEST_LITERAL(init_type, json)\
	do {\
		lept_value val;\
		lept_init(&val);\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json));\
		EXPECT_EQ_INT(init_type, lept_get_type(&val));\
		lept_free(&val);\
	}while(0)

//TEST_NUMBER用来测试数值型类型，CH02新增部分
#define TEST_NUMBER(expt, json)\
	do {\
		lept_value val;\
		lept_init(&val);\
        /* 测试是否解析成功 */                        \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json)); \
		/* 测试是否解析出正确数值类型 */                        \
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&val));\
		/* 测试是否解析出正确数值 */                        \
		EXPECT_EQ_DOUBLE(expt, lept_get_number(&val));\
		lept_free(&val);\
	}while(0)

//TEST_STRING用来测试字符串类型，CH03新增部分
#define TEST_STRING(expt, json)\
	do {\
		lept_value val;\
		lept_init(&val);\
		/* 测试是否解析成功 */                        \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json));\
		/* 测试是否解析出正确字符串类型 */                     \
		EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&val));\
		/* 测试是否解析出正确字符串值 */                  \
		EXPECT_EQ_STRING(expt, lept_get_string(&val), lept_get_string_length(&val));\
		lept_free(&val);\
	}while(0)

//由于TEST_ERROR与TEST_LITERAL基本一致，只是第一个参数有所不同
//错误解析后的JSON对象的所有类型都是LEPT_ERROR,但是解析错误的原因却有多种可能
#define TEST_ERROR(errRetType, json)\
	do {\
		lept_value val;\
		EXPECT_EQ_INT(errRetType, lept_parse(&val, json));\
		EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&val));\
	}while(0)
//TIPS：  TEST_LITERAL以及TEST_ERROR完全可以用三个参数进行统一化。
//但是之后写测试用例的时候，都需要输入三个参数，有点得不偿失。

//----------各种类型的单元测试部分-----------
//将CH01的null, true, false等单元测试进行重构，简化代码
static void test_parse_literal()
{
	TEST_LITERAL(LEPT_NULL, " null ");
	TEST_LITERAL(LEPT_TRUE, " true ");
	TEST_LITERAL(LEPT_FALSE, " false ");
}

//测试数值型
static void test_parse_number()
{
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1415, "3.1415");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1E10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1E012, "1E012");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	//下溢出
	TEST_NUMBER(0.0, "1e-10000");
	//----------- 边界值检测 --------
	TEST_NUMBER(5e-324, "5e-324");
	TEST_NUMBER(1.7976E308, "1.7976E308");
	TEST_NUMBER((double)12345678901234567890, "12345678901234567090");
	//大于1的最小浮点数
	TEST_NUMBER(1.0000000000000001, "1.0000000000000001");
	//绝对值最小的浮点数（>0）
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	//绝对值最大的浮点数
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	//绝对值最小的浮点数
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	//最大的浮点数
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

//测试string型解析
static void test_parse_string()
{
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	//测试Unicode码点U+0024
	TEST_STRING("\x24", "\"\\u0024\"");         
	//测试Unicode码点U+00A2
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");
	//测试Unicode码点U+20AC
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); 
	//测试Unicode码点U+1D11E
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); 
	//测试Unicode码点U+1D11E
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  
}


#define TEST_ARRAY_PARTIAL(exptVal, actu, size) \
		do{\
			EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&exptVal, actu));\
			EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&exptVal));\
			EXPECT_EQ_SIZE_T(size, lept_get_array_size(&exptVal));\
		}while(0)

//测试array型解析
static void test_parse_array()
{
	size_t i, j;
	lept_value val;

	lept_init(&val);
	TEST_ARRAY_PARTIAL(val, "[ ]", 0);
	lept_free(&val);

	lept_init(&val);
	TEST_ARRAY_PARTIAL(val, "[ false , true , 123 , \"abc\" , [ \"\\\"\\\"\", \"fs\" ] ]", 5);
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&val, 0)));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&val, 1)));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&val, 2)));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&val, 3)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_array_element(&val, 2)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&val, 3)), lept_get_string_length(lept_get_array_element(&val, 3)));
	lept_value* a1 = lept_get_array_element(&val, 4);
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a1));
	EXPECT_EQ_SIZE_T(2, lept_get_array_size(a1));
	lept_value* b1 = lept_get_array_element(a1, 0);
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(b1));
	EXPECT_EQ_STRING("\"\"", lept_get_string(b1), lept_get_string_length(b1));
	lept_value* b2 = lept_get_array_element(a1, 1);
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(b2));
	EXPECT_EQ_STRING("fs", lept_get_string(b2), lept_get_string_length(b2));
	lept_free(&val);

	lept_init(&val);
	TEST_ARRAY_PARTIAL(val, "[ null , false , true , 123 , \"abc\" ]", 5);
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_array_element(&val, 0)));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&val, 1)));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&val, 2)));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&val, 3)));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&val, 4)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_array_element(&val, 3)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&val, 4)), lept_get_string_length(lept_get_array_element(&val, 4)));
	lept_free(&val);

	lept_init(&val);
	TEST_ARRAY_PARTIAL(val, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", 4);
	for(i = 0; i < 4; i++)
	{
		lept_value* a = lept_get_array_element(&val, i);
		EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a));
		EXPECT_EQ_SIZE_T(i, lept_get_array_size(a));
		for(j = 0; j < i; j++)
		{
			lept_value* e = lept_get_array_element(a, j);
			EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
			EXPECT_EQ_DOUBLE((double)j, lept_get_number(e));
		}
	}
	lept_free(&val);
}

#define TEST_OBJECT_PARTIAL(exptVal, actu, size) \
		do{\
			EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&exptVal, actu));\
			EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&exptVal));\
			EXPECT_EQ_SIZE_T(size, lept_get_object_size(&exptVal));\
		}while(0)

//测试object类型解析
static void test_parse_object()
{
	lept_value val;
	size_t i;
	lept_init(&val);
	TEST_OBJECT_PARTIAL(val, " { } ", 0);
	lept_free(&val);

	lept_init(&val);
	TEST_OBJECT_PARTIAL(val,
						" { "
						"\"n\" : null , "
						"\"f\" : false , "
						"\"t\" : true , "
						"\"i\" : 123 , "
						"\"s\" : \"abc\", "
						"\"a\" : [ 1, 2, 3 ],"
						"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
						" } ", 7);
	EXPECT_EQ_STRING("n", lept_get_object_key(&val, 0), lept_get_object_key_length(&val, 0));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_object_value(&val, 0)));
	EXPECT_EQ_STRING("f", lept_get_object_key(&val, 1), lept_get_object_key_length(&val, 1));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_object_value(&val, 1)));
	EXPECT_EQ_STRING("t", lept_get_object_key(&val, 2), lept_get_object_key_length(&val, 2));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_object_value(&val, 2)));
	EXPECT_EQ_STRING("i", lept_get_object_key(&val, 3), lept_get_object_key_length(&val, 3));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_object_value(&val, 3)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_object_value(&val, 3)));
	EXPECT_EQ_STRING("s", lept_get_object_key(&val, 4), lept_get_object_key_length(&val, 4));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_object_value(&val, 4)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_object_value(&val, 4)), lept_get_string_length(lept_get_object_value(&val, 4)));
	EXPECT_EQ_STRING("a", lept_get_object_key(&val, 5), lept_get_object_key_length(&val, 5));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(lept_get_object_value(&val, 5)));
	EXPECT_EQ_SIZE_T(3, lept_get_array_size(lept_get_object_value(&val, 5)));
	for(i = 0; i < 3; i++)
	{
		lept_value* e = lept_get_array_element(lept_get_object_value(&val, 5), i);
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
		EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(e));
	}
	EXPECT_EQ_STRING("o", lept_get_object_key(&val, 6), lept_get_object_key_length(&val, 6));
	{
		lept_value* o = lept_get_object_value(&val, 6);
		EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(o));
		for(i = 0; i < 3; i++)
		{
			lept_value* ov = lept_get_object_value(o, i);
			EXPECT_EQ_INT('1' + i,  lept_get_object_key(o, i)[0]);
			EXPECT_EQ_SIZE_T(1, lept_get_object_key_length(o, i));
			EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(ov));
			EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(ov));
		}
	}
	lept_free(&val);
}

//测试正确解析出类型的JSON元素
static void test_parse_ok()
{
	//测试null, 以及boolean
	test_parse_literal();
	//测试数值
	test_parse_number();
	//测试字符串
	test_parse_string();
	//测试数组
	test_parse_array();
#if CH06
	//测试对象
	test_parse_object();
#endif
}

//测试期待值的情况，这种情况都是只有空白符的情况
static void test_parse_expect_value()
{
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

//测试不合理的输入
static void test_parse_invalid_value()
{
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "tr");
	//不合理的数值
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
	//在小数点之前，至少有一个数字
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");

	//NOTE: int a[] = {1,2,};  在C语言可以正确解析为int a[2] = {1, 2};
	//但是在JSON文本中是错误的数组文本
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "[1,]");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "[\"a\", nul]");

}

//测试不止是是单种类型
static void test_parse_root_not_singular()
{
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "truex ");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "false  null x");
	//0之后，应该是小数点或者没有值
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

//测试新的返回错误，解析的数值过大
static void test_parse_number_too_big()
{
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

//测试缺少后一个引号的情况
static void test_parse_missing_quotation_mark()
{
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

//测试出现不合法的转义字符的情况
static void test_parse_invalid_string_escape()
{
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

//测试出现不合法的字符串的字符的情况
static void test_parse_invalid_string_char()
{
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

//测试出现不合法的16进制数的情况
static void test_parse_invalid_unicode_hex()
{
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

//测试出现不合法的代理对的情况
static void test_parse_invalid_unicode_surrogate()
{
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

//测试解析数组时，缺少逗号，或者方括号的情况
static void test_parse_miss_comma_or_square_bracket()
{
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

//测试解析对象时，缺少关键字的情况
static void test_parse_miss_key()
{
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{1:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{true:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{false:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{null:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{[]:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{{}:1,");
	TEST_ERROR(LEPT_PARSE_MISS_KEY, "{\"a\":1,");
}

//测试解析对象时，缺少冒号:的情况
static void test_parse_miss_colon()
{
	TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}");
	TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

//测试解析对象时，缺少逗号,或者花括号的情况
static void test_parse_miss_comma_or_curly_bracket()
{
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

//总测试JSON解析器的解析部分
static void test_parse()
{
	test_parse_ok();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	test_parse_miss_comma_or_square_bracket();
#if CH06
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();
#endif
}

//测试接入null的宏定义
#define TEST_ACCESS_NULL(expt) \
	do {\
		lept_value val;\
		lept_init(&val);\
		/* 这是是会产生内存泄漏的 */  \
		/*lept_set_string(&val, "a", 1);*/\
		lept_set_null(&val);\
		EXPECT_EQ_INT(expt, lept_get_type(&val));\
		lept_free(&val);\
	}while(0)
//测试接入null元素
static void test_access_null()
{
	TEST_ACCESS_NULL(LEPT_NULL);
}

//判断两个boolean元素是否相等
#define EXPECT_EQ_BOOLEAN(res, expt, actual)\
	EXPECT_EQ_BASE((res), expt, actual, "%s")
//测试接入boolean的宏定义
#define TEST_ACCESS_BOOLEAN(expt_type, expt, n)\
	do {\
		lept_value val;\
		lept_init(&val);\
		/* 这是是会产生内存泄漏的 */  \
		/*lept_set_string(&val, "a", 1);*/\
		lept_set_boolean(&val, n);\
		EXPECT_EQ_BOOLEAN((expt_type == lept_get_boolean(&val)), expt,\
			(n == 1 ? "true" : "false"));\
	}while(0)
//测试接入boolean元素
static void test_access_boolean()
{
	TEST_ACCESS_BOOLEAN(LEPT_TRUE, "true", 1);
	TEST_ACCESS_BOOLEAN(LEPT_FALSE, "false", 0);
}

//测试接入number的宏定义
#define TEST_ACCESS_NUMBER(expt) \
	do {\
		lept_value val;\
		lept_init(&val);\
		/* 这是是会产生内存泄漏的 */  \
		/*lept_set_string(&val, "a", 1);*/\
		lept_set_number(&val, expt);\
		EXPECT_EQ_DOUBLE(expt, lept_get_number(&val));\
		lept_free(&val);\
	}while(0)
//测试接入number
static void test_access_number()
{
	//NOTE: C语言可以解析“05.6E25”为5.6E25
	TEST_ACCESS_NUMBER(05.6E25);
	TEST_ACCESS_NUMBER(1234.5);
}

//测试接入string的宏定义
#define TEST_ACCESS_STRING(expt, len) \
	do{\
		lept_value val;\
		lept_init(&val);\
		lept_set_string(&val, expt, len);\
		EXPECT_EQ_STRING(expt, lept_get_string(&val), lept_get_string_length(&val));\
		lept_free(&val);\
	}while(0)
//测试接入string元素
static void test_access_string()
{
	TEST_ACCESS_STRING("", 0);
	TEST_ACCESS_STRING("Hello", 5);
}

//总测试JSON解析器的接入部分
static void test_access()
{
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
}

//总测试  测试：解析部分+接入部分
static void test()
{
	//测试JSON解析器的解析部分
	test_parse();
	//测试JSON解析器的接入部分
	test_access();
}

int main()
{
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//总测试器
	test();
	printf("%d/%d  (%3.2f%%) passed\n", test_pass, test_cnt, test_pass*100.0 / test_cnt);
	
	return main_ret;
}