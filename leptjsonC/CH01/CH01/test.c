#include "leptjson.h"
#include <stdio.h>

//total number of test: 20

//�������ķ���ֵ��0Ϊ�������أ���0���ڴ���
static int main_ret = 0;
//�������ݵ�����
static int test_cnt = 0;
//�������ݵ�ͨ������
static int test_pass = 0;

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

#define EXPECT_EQ_INT(expt, actu) EXPECT_EQ_BASE((expt) == (actu), expt, actu, "%d")

//����null���ͽ���
static void test_parse_null()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " null "));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

//����true����
static void test_parse_true()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " true "));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

//����false����
static void test_parse_false()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " false "));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

//�����ڴ�ֵ������������������ֻ�пհ׷������
static void test_parse_expect_value()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));

	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));
}

//���Բ����������
static void test_parse_invalid_value()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));

	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, " t  r"));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));
}

//���Բ�ֹ���ǵ�������
static void test_parse_root_not_singular()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));

	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "   truex "));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));

	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "   false  nul x"));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));
}
//�ܲ���JSON������
static void test_parse()
{
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
}

int main()
{
	test_parse();
	printf("%d/%d  (%3.2f%%) passed\n", test_pass, test_cnt, test_pass*100.0 / test_cnt);
	return main_ret;
}