// _WINDOWS���ڴ�й©��鿪��
#define _WINDOWS 1
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "leptjson.h"
#include <stdio.h>
#include <string.h>   //memcmp

//total number of previous test: 289
//total number of current test:  358

//ȥ��CH05 �������

//CH06��ΪCH05�����Ĳ��������Ŀ��أ�
//1��ʾ������Щ�µĲ���������0��ʾ�رգ�
//���������ױȽ���֮ǰһ�µĲ���������ͬ��
#define CH06 1


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
//���ǵĹ�ͬ�㶼�ǽ����ɹ���������LEPT_PARSE_OK;���ǽ���������Ԫ�����ʹ��ڲ�ͬ
#define TEST_LITERAL(init_type, json)\
	do {\
		lept_value val;\
		lept_init(&val);\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json));\
		EXPECT_EQ_INT(init_type, lept_get_type(&val));\
		lept_free(&val);\
	}while(0)

//TEST_NUMBER����������ֵ�����ͣ�CH02��������
#define TEST_NUMBER(expt, json)\
	do {\
		lept_value val;\
		lept_init(&val);\
        /* �����Ƿ�����ɹ� */                        \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json)); \
		/* �����Ƿ��������ȷ��ֵ���� */                        \
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&val));\
		/* �����Ƿ��������ȷ��ֵ */                        \
		EXPECT_EQ_DOUBLE(expt, lept_get_number(&val));\
		lept_free(&val);\
	}while(0)

//TEST_STRING���������ַ������ͣ�CH03��������
#define TEST_STRING(expt, json)\
	do {\
		lept_value val;\
		lept_init(&val);\
		/* �����Ƿ�����ɹ� */                        \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&val, json));\
		/* �����Ƿ��������ȷ�ַ������� */                     \
		EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&val));\
		/* �����Ƿ��������ȷ�ַ���ֵ */                  \
		EXPECT_EQ_STRING(expt, lept_get_string(&val), lept_get_string_length(&val));\
		lept_free(&val);\
	}while(0)

//����TEST_ERROR��TEST_LITERAL����һ�£�ֻ�ǵ�һ������������ͬ
//����������JSON������������Ͷ���LEPT_ERROR,���ǽ��������ԭ��ȴ�ж��ֿ���
#define TEST_ERROR(errRetType, json)\
	do {\
		lept_value val;\
		EXPECT_EQ_INT(errRetType, lept_parse(&val, json));\
		EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&val));\
	}while(0)
//TIPS��  TEST_LITERAL�Լ�TEST_ERROR��ȫ������������������ͳһ����
//����֮��д����������ʱ�򣬶���Ҫ���������������е�ò���ʧ��

//----------�������͵ĵ�Ԫ���Բ���-----------
//��CH01��null, true, false�ȵ�Ԫ���Խ����ع����򻯴���
static void test_parse_literal()
{
	TEST_LITERAL(LEPT_NULL, " null ");
	TEST_LITERAL(LEPT_TRUE, " true ");
	TEST_LITERAL(LEPT_FALSE, " false ");
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
			EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&exptVal, actu));\
			EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&exptVal));\
			EXPECT_EQ_SIZE_T(size, lept_get_array_size(&exptVal));\
		}while(0)

//����array�ͽ���
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

//����object���ͽ���
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
#if CH06
	//���Զ���
	test_parse_object();
#endif
}

//�����ڴ�ֵ������������������ֻ�пհ׷������
static void test_parse_expect_value()
{
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

//���Բ����������
static void test_parse_invalid_value()
{
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "tr");
	//���������ֵ
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
	//��С����֮ǰ��������һ������
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");

	//NOTE: int a[] = {1,2,};  ��C���Կ�����ȷ����Ϊint a[2] = {1, 2};
	//������JSON�ı����Ǵ���������ı�
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "[1,]");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "[\"a\", nul]");

}

//���Բ�ֹ���ǵ�������
static void test_parse_root_not_singular()
{
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "truex ");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "false  null x");
	//0֮��Ӧ����С�������û��ֵ
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

//�����µķ��ش��󣬽�������ֵ����
static void test_parse_number_too_big()
{
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

//����ȱ�ٺ�һ�����ŵ����
static void test_parse_missing_quotation_mark()
{
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

//���Գ��ֲ��Ϸ���ת���ַ������
static void test_parse_invalid_string_escape()
{
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

//���Գ��ֲ��Ϸ����ַ������ַ������
static void test_parse_invalid_string_char()
{
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

//���Գ��ֲ��Ϸ���16�����������
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

//���Գ��ֲ��Ϸ��Ĵ���Ե����
static void test_parse_invalid_unicode_surrogate()
{
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

//���Խ�������ʱ��ȱ�ٶ��ţ����߷����ŵ����
static void test_parse_miss_comma_or_square_bracket()
{
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

//���Խ�������ʱ��ȱ�ٹؼ��ֵ����
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

//���Խ�������ʱ��ȱ��ð��:�����
static void test_parse_miss_colon()
{
	TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}");
	TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

//���Խ�������ʱ��ȱ�ٶ���,���߻����ŵ����
static void test_parse_miss_comma_or_curly_bracket()
{
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
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
#if CH06
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();
#endif
}

//���Խ���null�ĺ궨��
#define TEST_ACCESS_NULL(expt) \
	do {\
		lept_value val;\
		lept_init(&val);\
		/* �����ǻ�����ڴ�й©�� */  \
		/*lept_set_string(&val, "a", 1);*/\
		lept_set_null(&val);\
		EXPECT_EQ_INT(expt, lept_get_type(&val));\
		lept_free(&val);\
	}while(0)
//���Խ���nullԪ��
static void test_access_null()
{
	TEST_ACCESS_NULL(LEPT_NULL);
}

//�ж�����booleanԪ���Ƿ����
#define EXPECT_EQ_BOOLEAN(res, expt, actual)\
	EXPECT_EQ_BASE((res), expt, actual, "%s")
//���Խ���boolean�ĺ궨��
#define TEST_ACCESS_BOOLEAN(expt_type, expt, n)\
	do {\
		lept_value val;\
		lept_init(&val);\
		/* �����ǻ�����ڴ�й©�� */  \
		/*lept_set_string(&val, "a", 1);*/\
		lept_set_boolean(&val, n);\
		EXPECT_EQ_BOOLEAN((expt_type == lept_get_boolean(&val)), expt,\
			(n == 1 ? "true" : "false"));\
	}while(0)
//���Խ���booleanԪ��
static void test_access_boolean()
{
	TEST_ACCESS_BOOLEAN(LEPT_TRUE, "true", 1);
	TEST_ACCESS_BOOLEAN(LEPT_FALSE, "false", 0);
}

//���Խ���number�ĺ궨��
#define TEST_ACCESS_NUMBER(expt) \
	do {\
		lept_value val;\
		lept_init(&val);\
		/* �����ǻ�����ڴ�й©�� */  \
		/*lept_set_string(&val, "a", 1);*/\
		lept_set_number(&val, expt);\
		EXPECT_EQ_DOUBLE(expt, lept_get_number(&val));\
		lept_free(&val);\
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
		lept_value val;\
		lept_init(&val);\
		lept_set_string(&val, expt, len);\
		EXPECT_EQ_STRING(expt, lept_get_string(&val), lept_get_string_length(&val));\
		lept_free(&val);\
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
	printf("%d/%d  (%3.2f%%) passed\n", test_pass, test_cnt, test_pass*100.0 / test_cnt);
	
	return main_ret;
}