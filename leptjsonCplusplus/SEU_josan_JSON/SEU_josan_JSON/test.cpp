/*
 * ---------------------------------------------------
 *  Copyright (c) 2017 josan All rights reserved.
 * ---------------------------------------------------
 *
 *               �����ߣ� Josan
 *             ����ʱ�䣺 2017/9/28 16:21:19
 */
 // _WINDOWS���ڴ�й©��鿪��
#define _WINDOWS 1
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <iostream>
#include "json.h"
#include <cstring>   //memcmp
using std::cerr;
using std::cout;
using std::endl;
using namespace SEU_josan_JSON;
 //total number of previous test: 358
 //total number of current test:  423

 //�������ķ���ֵ��0Ϊ�������أ���0���ڴ���
static int main_ret = 0;
//�������ݵ�����
static int test_cnt = 0;
//�������ݵ�ͨ������
static int test_pass = 0;

//------------------CH04���õ������к궨��------------
//���к궨��ȽϵĻ�������ʽ
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

//�Ƚ����������Ƿ����
#define EXPECT_EQ_INT(expt, actu) EXPECT_EQ_BASE((expt) == (actu), expt, actu, "%d")
//�Ƚ������������Ƿ����
#define EXPECT_EQ_DOUBLE(expt, actu) EXPECT_EQ_BASE((expt) == (actu), expt, actu, "%.17g")
//�Ƚ������ַ����Ƿ����
#define EXPECT_EQ_STRING(expt, actu, len) \
	EXPECT_EQ_BASE(sizeof(expt) - 1 == len && 0 == memcmp(expt, actu, len), expt, actu, "%s")

//�Ƚ���������Ĵ�С�Ƿ����
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expt, actu) \
		EXPECT_EQ_BASE((expt) == (actu), (size_t)expt, (size_t)actu, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expt, actu) \
		EXPECT_EQ_BASE((expt) == (actu), (size_t)expt, (size_t)actu, "%zu")
#endif

//�ع����룬��ԭ���Ĳ��Լ� TEST_LITERAL������������null, true, false
//���ǵĹ�ͬ�㶼�ǽ����ɹ���������JSON_PARSE_OK;���ǽ���������Ԫ�����ʹ��ڲ�ͬ
#define TEST_LITERAL(init_type, json)\
	do {\
		json_value val;\
		val.init();\
		EXPECT_EQ_INT(JSON_PARSE_OK, val.json_parse(json));\
		EXPECT_EQ_INT(init_type, val.json_get_type());\
		val.json_free();\
	}while(0)

//TEST_NUMBER����������ֵ������
#define TEST_NUMBER(expt, json)\
	do {\
		json_value val;\
		val.init();\
        /* �����Ƿ�����ɹ� */                        \
		EXPECT_EQ_INT(JSON_PARSE_OK, val.json_parse(json)); \
		/* �����Ƿ��������ȷ��ֵ���� */                        \
		EXPECT_EQ_INT(JSON_NUMBER, val.json_get_type());\
		/* �����Ƿ��������ȷ��ֵ */                        \
		EXPECT_EQ_DOUBLE(expt, val.json_get_number());\
		val.json_free();\
	}while(0)

//TEST_STRING���������ַ�������
#define TEST_STRING(expt, json)\
	do {\
		json_value val;\
		val.init();\
		/* �����Ƿ�����ɹ� */                        \
		EXPECT_EQ_INT(JSON_PARSE_OK, val.json_parse(json));\
		/* �����Ƿ��������ȷ�ַ������� */                     \
		EXPECT_EQ_INT(JSON_STRING, val.json_get_type());\
		/* �����Ƿ��������ȷ�ַ���ֵ */                  \
		EXPECT_EQ_STRING(expt, val.json_get_string(), val.json_get_string_length());\
		val.json_free();\
	}while(0)

//����TEST_ERROR��TEST_LITERAL����һ�£�ֻ�ǵ�һ������������ͬ
//����������JSON������������Ͷ���JSON_ERROR,���ǽ��������ԭ��ȴ�ж��ֿ���
#define TEST_ERROR(errRetType, json)\
	do {\
		json_value val;\
		EXPECT_EQ_INT(errRetType, val.json_parse(json));\
		EXPECT_EQ_INT(JSON_ERROR, val.json_get_type());\
	}while(0)
//TIPS��  TEST_LITERAL�Լ�TEST_ERROR��ȫ������������������ͳһ����
//����֮��д����������ʱ�򣬶���Ҫ���������������е�ò���ʧ��

//----------�������͵ĵ�Ԫ���Բ���-----------
//��CH01��null, true, false�ȵ�Ԫ���Խ����ع����򻯴���
static void test_parse_literal()
{
	TEST_LITERAL(JSON_NULL, " null ");
	TEST_LITERAL(JSON_TRUE, " true ");
	TEST_LITERAL(JSON_FALSE, " false ");
}

//������ֵ��
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
	//�����
	TEST_NUMBER(0.0, "1e-10000");
	//----------- �߽�ֵ��� --------
	TEST_NUMBER(5e-324, "5e-324");
	TEST_NUMBER(1.7976E308, "1.7976E308");
	TEST_NUMBER((double)12345678901234567890, "12345678901234567090");
	//����1����С������
	TEST_NUMBER(1.0000000000000001, "1.0000000000000001");
	//����ֵ��С�ĸ�������>0��
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	//����ֵ���ĸ�����
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	//����ֵ��С�ĸ�����
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	//���ĸ�����
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

//����string�ͽ���
static void test_parse_string()
{
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	//CH07 add this test
	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	//����Unicode���U+0024
	TEST_STRING("\x24", "\"\\u0024\"");
	//����Unicode���U+00A2
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");
	//����Unicode���U+20AC
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"");
	//����Unicode���U+1D11E
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");
	//����Unicode���U+1D11E
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");
}


#define TEST_ARRAY_PARTIAL(exptVal, actu, size) \
		do{\
			EXPECT_EQ_INT(JSON_PARSE_OK, exptVal.json_parse(actu));\
			EXPECT_EQ_INT(JSON_ARRAY, exptVal.json_get_type());\
			EXPECT_EQ_SIZE_T(size, exptVal.json_get_array_size());\
		}while(0)

//����array�ͽ���
static void test_parse_array()
{
	size_t i, j;
	json_value val;

	val.init();
	TEST_ARRAY_PARTIAL(val, "[ ]", 0);
	val.json_free();

	val.init();
	TEST_ARRAY_PARTIAL(val, "[ false , true , 123 , \"abc\" , [ \"\\\"\\\"\", \"fs\" ] ]", 5);
	EXPECT_EQ_INT(JSON_FALSE, val.json_get_array_element(0)->json_get_type());
	EXPECT_EQ_INT(JSON_TRUE, val.json_get_array_element(1)->json_get_type());
	EXPECT_EQ_INT(JSON_NUMBER, val.json_get_array_element(2)->json_get_type());
	EXPECT_EQ_INT(JSON_STRING, val.json_get_array_element(3)->json_get_type());
	EXPECT_EQ_DOUBLE(123.0, val.json_get_array_element(2)->json_get_number());
	EXPECT_EQ_STRING("abc", val.json_get_array_element(3)->json_get_string(), val.json_get_array_element(3)->json_get_string_length());
	json_value* a1 = val.json_get_array_element(4);
	EXPECT_EQ_INT(JSON_ARRAY, a1->json_get_type());
	EXPECT_EQ_SIZE_T(2, a1->json_get_array_size());
	json_value* b1 = a1->json_get_array_element(0);
	EXPECT_EQ_INT(JSON_STRING, b1->json_get_type());
	EXPECT_EQ_STRING("\"\"", b1->json_get_string(), b1->json_get_string_length());
	json_value* b2 = a1->json_get_array_element(1);
	EXPECT_EQ_INT(JSON_STRING, b2->json_get_type());
	EXPECT_EQ_STRING("fs", b2->json_get_string(), b2->json_get_string_length());
	val.json_free();

	val.init();
	TEST_ARRAY_PARTIAL(val, "[ null , false , true , 123 , \"abc\" ]", 5);
	EXPECT_EQ_INT(JSON_NULL, val.json_get_array_element(0)->json_get_type());
	EXPECT_EQ_INT(JSON_FALSE, val.json_get_array_element(1)->json_get_type());
	EXPECT_EQ_INT(JSON_TRUE, val.json_get_array_element(2)->json_get_type());
	EXPECT_EQ_INT(JSON_NUMBER, val.json_get_array_element(3)->json_get_type());
	EXPECT_EQ_INT(JSON_STRING, val.json_get_array_element(4)->json_get_type());
	EXPECT_EQ_DOUBLE(123.0, val.json_get_array_element(3)->json_get_number());
	EXPECT_EQ_STRING("abc", val.json_get_array_element(4)->json_get_string(), val.json_get_array_element(4)->json_get_string_length());
	val.json_free();

	val.init();
	TEST_ARRAY_PARTIAL(val, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", 4);
	for(i = 0; i < 4; i++)
	{
		json_value* a = val.json_get_array_element( i);
		EXPECT_EQ_INT(JSON_ARRAY, a->json_get_type());
		EXPECT_EQ_SIZE_T(i, a->json_get_array_size());
		for(j = 0; j < i; j++)
		{
			json_value* e = a->json_get_array_element(j);
			EXPECT_EQ_INT(JSON_NUMBER, e->json_get_type());
			EXPECT_EQ_DOUBLE((double)j, e->json_get_number());
		}
	}
	val.json_free();
}

#define TEST_OBJECT_PARTIAL(exptVal, actu, size) \
		do{\
			EXPECT_EQ_INT(JSON_PARSE_OK, exptVal.json_parse(actu));\
			EXPECT_EQ_INT(JSON_OBJECT, exptVal.json_get_type());\
			EXPECT_EQ_SIZE_T(size, exptVal.json_get_object_size());\
		}while(0)

//����object���ͽ���
static void test_parse_object()
{
	json_value val;
	size_t i;
	val.init();
	TEST_OBJECT_PARTIAL(val, " { } ", 0);
	val.json_free();

	val.init();
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
	EXPECT_EQ_STRING("n", val.json_get_object_key(0), val.json_get_object_key_length(0));
	EXPECT_EQ_INT(JSON_NULL, val.json_get_object_value(0)->json_get_type());
	EXPECT_EQ_STRING("f", val.json_get_object_key( 1), val.json_get_object_key_length( 1));
	EXPECT_EQ_INT(JSON_FALSE, val.json_get_object_value(1)->json_get_type());
	EXPECT_EQ_STRING("t", val.json_get_object_key( 2), val.json_get_object_key_length( 2));
	EXPECT_EQ_INT(JSON_TRUE, val.json_get_object_value(2)->json_get_type());
	EXPECT_EQ_STRING("i", val.json_get_object_key( 3), val.json_get_object_key_length( 3));
	EXPECT_EQ_INT(JSON_NUMBER, val.json_get_object_value(3)->json_get_type());
	EXPECT_EQ_DOUBLE(123.0, val.json_get_object_value(3)->json_get_number());
	EXPECT_EQ_STRING("s", val.json_get_object_key( 4), val.json_get_object_key_length( 4));
	EXPECT_EQ_INT(JSON_STRING, val.json_get_object_value(4)->json_get_type());
	EXPECT_EQ_STRING("abc", val.json_get_object_value(4)->json_get_string(), val.json_get_object_value(4)->json_get_string_length());
	EXPECT_EQ_STRING("a", val.json_get_object_key( 5), val.json_get_object_key_length( 5));
	EXPECT_EQ_INT(JSON_ARRAY, val.json_get_object_value(5)->json_get_type());
	EXPECT_EQ_SIZE_T(3, val.json_get_object_value(5)->json_get_array_size());
	for(i = 0; i < 3; i++)
	{
		json_value* e = val.json_get_object_value(5)->json_get_array_element(i);
		EXPECT_EQ_INT(JSON_NUMBER, e->json_get_type());
		EXPECT_EQ_DOUBLE(i + 1.0, e->json_get_number());
	}
	EXPECT_EQ_STRING("o", val.json_get_object_key( 6), val.json_get_object_key_length( 6));
	{
		json_value* o = val.json_get_object_value( 6);
		EXPECT_EQ_INT(JSON_OBJECT, o->json_get_type());
		for(i = 0; i < 3; i++)
		{
			json_value* ov = o->json_get_object_value( i);
			EXPECT_EQ_INT('1' + i, o->json_get_object_key(i)[0]);
			EXPECT_EQ_SIZE_T(1, o->json_get_object_key_length( i));
			EXPECT_EQ_INT(JSON_NUMBER, ov->json_get_type());
			EXPECT_EQ_DOUBLE(i + 1.0, ov->json_get_number());
		}
	}
	val.json_free();
}

//������ȷ���������͵�JSONԪ��
static void test_parse_ok()
{
	//����null, �Լ�boolean
	test_parse_literal();
	//������ֵ
	test_parse_number();
	//�����ַ���
	test_parse_string();
	//��������
	test_parse_array();
	//���Զ���
	test_parse_object();
}

//�����ڴ�ֵ������������������ֻ�пհ׷������
static void test_parse_expect_value()
{
	TEST_ERROR(JSON_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(JSON_PARSE_EXPECT_VALUE, " ");
}

//���Բ����������
static void test_parse_invalid_value()
{
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "tr");
	//���������ֵ
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
	//��С����֮ǰ��������һ������
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, ".123");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "1.");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nan");
	//NOTE: int a[] = {1,2,};  ��C���Կ�����ȷ����Ϊint a[2] = {1, 2};
	//������JSON�ı����Ǵ���������ı�
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "[1,]");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "[\"a\", nul]");

}

//���Բ�ֹ���ǵ�������
static void test_parse_root_not_singular()
{
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "null x");
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "truex ");
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "false  null x");
	//0֮��Ӧ����С�������û��ֵ
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0123");
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

//�����µķ��ش��󣬽�������ֵ����
static void test_parse_number_too_big()
{
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

//����ȱ�ٺ�һ�����ŵ����
static void test_parse_missing_quotation_mark()
{
	TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

//���Գ��ֲ��Ϸ���ת���ַ������
static void test_parse_invalid_string_escape()
{
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

//���Գ��ֲ��Ϸ����ַ������ַ������
static void test_parse_invalid_string_char()
{
	TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

//���Գ��ֲ��Ϸ���16�����������
static void test_parse_invalid_unicode_hex()
{
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

//���Գ��ֲ��Ϸ��Ĵ���Ե����
static void test_parse_invalid_unicode_surrogate()
{
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

//���Խ�������ʱ��ȱ�ٶ��ţ����߷����ŵ����
static void test_parse_miss_comma_or_square_bracket()
{
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

//���Խ�������ʱ��ȱ�ٹؼ��ֵ����
static void test_parse_miss_key()
{
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{1:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{true:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{false:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{null:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{[]:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{{}:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{\"a\":1,");
}

//���Խ�������ʱ��ȱ��ð��:�����
static void test_parse_miss_colon()
{
	TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
	TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

//���Խ�������ʱ��ȱ�ٶ���,���߻����ŵ����
static void test_parse_miss_comma_or_curly_bracket()
{
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

//�ܲ���JSON�������Ľ�������
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
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();
}

#define TEST_STRINGIFY_BASE(json) \
		do{\
			json_value val;\
			char* json2;\
			size_t length;\
			val.init();\
			EXPECT_EQ_INT(JSON_PARSE_OK, val.json_parse(json));\
			json2 = val.json_stringify(&length);\
			EXPECT_EQ_STRING(json, json2, length);\
			val.json_free();\
			free(json2);\
		}while(0)

static void test_stringify_number()
{
	TEST_STRINGIFY_BASE("0");
	TEST_STRINGIFY_BASE("-0");
	TEST_STRINGIFY_BASE("1");
	TEST_STRINGIFY_BASE("-1");
	TEST_STRINGIFY_BASE("1.5");
	TEST_STRINGIFY_BASE("-1.5");
	TEST_STRINGIFY_BASE("3.25");
	TEST_STRINGIFY_BASE("1e+20");
	TEST_STRINGIFY_BASE("1.234e+20");
	TEST_STRINGIFY_BASE("1.234e-20");
	TEST_STRINGIFY_BASE("1.0000000000000002"); /* the smallest number > 1 */
	TEST_STRINGIFY_BASE("4.9406564584124654e-324"); /* minimum denormal */
	TEST_STRINGIFY_BASE("-4.9406564584124654e-324");
	TEST_STRINGIFY_BASE("2.2250738585072009e-308");  /* Max subnormal double */
	TEST_STRINGIFY_BASE("-2.2250738585072009e-308");
	TEST_STRINGIFY_BASE("2.2250738585072014e-308");  /* Min normal positive double */
	TEST_STRINGIFY_BASE("-2.2250738585072014e-308");
	TEST_STRINGIFY_BASE("1.7976931348623157e+308");  /* Max double */
	TEST_STRINGIFY_BASE("-1.7976931348623157e+308");
}

static void test_stringify_string()
{
	TEST_STRINGIFY_BASE("\"\"");
	TEST_STRINGIFY_BASE("\"Hello\"");
	TEST_STRINGIFY_BASE("\"Hello\\nWorld\"");
	TEST_STRINGIFY_BASE("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	TEST_STRINGIFY_BASE("\"Hello\\u0000World\"");
}

static void test_stringify_array()
{
	TEST_STRINGIFY_BASE("[]");
	TEST_STRINGIFY_BASE("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object()
{
	TEST_STRINGIFY_BASE("{}");
	TEST_STRINGIFY_BASE("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify()
{
	TEST_STRINGIFY_BASE("null");
	TEST_STRINGIFY_BASE("false");
	TEST_STRINGIFY_BASE("true");
	test_stringify_number();
	test_stringify_string();
	test_stringify_array();
	test_stringify_object();
}

//���Խ���null�ĺ궨��
#define TEST_ACCESS_NULL(expt) \
	do {\
		json_value val;\
		val.init();\
		/* �����ǻ�����ڴ�й©�� */  \
		/*json_set_string(&val, "a", 1);*/\
		val.json_set_null();\
		EXPECT_EQ_INT(expt, val.json_get_type());\
		val.json_free();\
	}while(0)
//���Խ���nullԪ��
static void test_access_null()
{
	TEST_ACCESS_NULL(JSON_NULL);
}

//�ж�����booleanԪ���Ƿ����
#define EXPECT_EQ_BOOLEAN(res, expt, actual)\
	EXPECT_EQ_BASE((res), expt, actual, "%s")
//���Խ���boolean�ĺ궨��
#define TEST_ACCESS_BOOLEAN(expt_type, expt, n)\
	do {\
		json_value val;\
		val.init();\
		/* �����ǻ�����ڴ�й©�� */  \
		/*json_set_string(&val, "a", 1);*/\
		val.json_set_boolean(n);\
		EXPECT_EQ_BOOLEAN((expt_type == val.json_get_boolean()), expt,\
			(n == 1 ? "true" : "false"));\
	}while(0)
//���Խ���booleanԪ��
static void test_access_boolean()
{
	TEST_ACCESS_BOOLEAN(JSON_TRUE, "true", JSON_TRUE);
	TEST_ACCESS_BOOLEAN(JSON_FALSE, "false", JSON_FALSE);
}

//���Խ���number�ĺ궨��
#define TEST_ACCESS_NUMBER(expt) \
	do {\
		json_value val;\
		val.init();\
		/* �����ǻ�����ڴ�й©�� */  \
		/*json_set_string(&val, "a", 1);*/\
		val.json_set_number(expt);\
		EXPECT_EQ_DOUBLE(expt, val.json_get_number());\
		val.json_free();\
	}while(0)
//���Խ���number
static void test_access_number()
{
	//NOTE: C���Կ��Խ�����05.6E25��Ϊ5.6E25
	TEST_ACCESS_NUMBER(05.6E25);
	TEST_ACCESS_NUMBER(1234.5);
}

//���Խ���string�ĺ궨��
#define TEST_ACCESS_STRING(expt, len) \
	do{\
		json_value val;\
		val.init();\
		val.json_set_string(expt, len);\
		EXPECT_EQ_STRING(expt, val.json_get_string(), val.json_get_string_length());\
		val.json_free();\
	}while(0)
//���Խ���stringԪ��
static void test_access_string()
{
	TEST_ACCESS_STRING("", 0);
	TEST_ACCESS_STRING("Hello", 5);
}

//�ܲ���JSON�������Ľ��벿��
static void test_access()
{
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
}

//�ܲ���  ���ԣ���������+���벿��
static void test()
{
	//����JSON�������Ľ�������
	test_parse();

	//����JSON�����������������ַ���������
	test_stringify();

	//����JSON�������Ľ��벿��
	test_access();
}

int main()
{
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//�ܲ�����
	test();

	cout << test_pass << "/" << test_cnt << "  (";
	//cout.width(4);
	cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
	cout.precision(2);
	cout << test_pass*100.0 / test_cnt << "%) passed" << endl;
	return main_ret;
}