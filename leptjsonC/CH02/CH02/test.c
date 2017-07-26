#include "leptjson.h"
#include <stdio.h>

//total number of previous test: 20
//total number of current test: 142

//CH02��ΪCH02�����Ĳ��������Ŀ��أ�
//1��ʾ������Щ�µĲ���������0��ʾ�رգ�
//���������ױȽ���֮ǰһ�µĲ���������ͬ��
#define CH02 1

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

//�ع����룬��ԭ���Ĳ��Լ� TEST_LITERAL������������null, true, false
//���ǵĹ�ͬ�㶼�ǽ����ɹ���������LEPT_PARSE_OK;���ǽ���������Ԫ�����ʹ��ڲ�ͬ
#define TEST_LITERAL(init_type, json)\
	do {\
		lept_value v;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(init_type, lept_get_type(&v));\
	}while(0)
//TEST_NUMBER����������ֵ�����ͣ�CH02��������
#define TEST_NUMBER(expt, json)\
	do {\
		lept_value v;\
        /* �����Ƿ�����ɹ� */                        \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
		/* �����Ƿ��������ȷ��ֵ���� */                        \
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		/* �����Ƿ��������ȷ��ֵ */                        \
		EXPECT_EQ_DOUBLE(expt, lept_get_number(&v));\
	}while(0)
//����TEST_ERROR��TEST_LITERAL����һ�£�ֻ�ǵ�һ������������ͬ
//����������JSON������������Ͷ���LEPT_ERROR,���ǽ��������ԭ��ȴ�ж��ֿ���
#define TEST_ERROR(errRetType, json)\
	do {\
		lept_value v;\
		EXPECT_EQ_INT(errRetType, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));\
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

#if CH02
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
#if CH02
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
#endif
}

//���Բ�ֹ���ǵ�������
static void test_parse_root_not_singular()
{
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "truex ");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "false  null x");
#if CH02
	//0֮��Ӧ����С�������û��ֵ
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}

#if CH02
//�����µķ��ش��󣬽�������ֵ����
static void test_parse_number_too_big()
{
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}
#endif

//������ȷ���������͵�JSONԪ��
static void test_parse_ok()
{
	//����null, �Լ�boolean
	test_parse_literal();
#if CH02
	//������ֵ
	test_parse_number();
#endif
}

//�ܲ���JSON������
static void test_parse()
{
	test_parse_ok();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
#if CH02
	test_parse_number_too_big();
#endif
}

int main()
{
	test_parse();
	printf("%d/%d  (%3.2f%%) passed\n", test_pass, test_cnt, test_pass*100.0 / test_cnt);
	return main_ret;
}