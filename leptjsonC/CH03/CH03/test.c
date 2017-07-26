#include "leptjson.h"
#include <stdio.h>
#include <string.h>   //memcmp

//total number of previous test: 142
//total number of current test:  177

//ȥ��CH02 �������

//CH03��ΪCH02�����Ĳ��������Ŀ��أ�
//1��ʾ������Щ�µĲ���������0��ʾ�رգ�
//���������ױȽ���֮ǰһ�µĲ���������ͬ��
#define CH03 1


//�������ķ���ֵ��0Ϊ�������أ���0���ڴ���
static int main_ret = 0;
//�������ݵ�����
static int test_cnt = 0;
//�������ݵ�ͨ������
static int test_pass = 0;

//------------------CH02���õ������к궨��------------
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

#if CH03
static void test_parse_string()
{
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}
#endif


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

#if CH03
//����ȱ�ٺ�һ�����ŵ����
static void test_parse_missing_quotation_mark()
{
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}
#endif

#if CH03
//���Գ��ֲ��Ϸ���ת���ַ������
static void test_parse_invalid_string_escape()
{
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}
#endif

#if CH03
//���Գ��ֲ��Ϸ����ַ������ַ������
static void test_parse_invalid_string_char()
{
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}
#endif

//������ȷ���������͵�JSONԪ��
static void test_parse_ok()
{
	//����null, �Լ�boolean
	test_parse_literal();
	//������ֵ
	test_parse_number();
#if CH03
	//�����ַ���
	test_parse_string();
#endif
}

//�ܲ���JSON�������Ľ�������
static void test_parse()
{
	test_parse_ok();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
#if CH03
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
#endif
}

//���Խ���null�ĺ궨��
#define TEST_ACCESS_NULL(expt) \
	do {\
		lept_value val;\
		lept_init(&val);\
		lept_set_string(&val, "a", 1);\
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
		lept_set_string(&val, "a", 1);\
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
		lept_set_string(&val, "a", 1);\
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
	}while(0)
//���Խ���stringԪ��
static void test_access_string()
{
	TEST_ACCESS_STRING("", 0);
	TEST_ACCESS_STRING("Hello", 5);
}

#if CH03
//�ܲ���JSON�������Ľ��벿��
static void test_access()
{
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
}
#endif

//�ܲ���  ���ԣ���������+���벿��
static void test()
{
	//����JSON�������Ľ�������
	test_parse();
#if CH03
	//����JSON�������Ľ��벿��
	test_access();
#endif
}

int main()
{
	//�ܲ�����
	test();
	printf("%d/%d  (%3.2f%%) passed\n", test_pass, test_cnt, test_pass*100.0 / test_cnt);
	return main_ret;
}