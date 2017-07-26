#include "leptjson.h"
#include <stdio.h>

//total number of test: 20

//主函数的返回值，0为正常返回，非0存在错误
static int main_ret = 0;
//测试数据的数量
static int test_cnt = 0;
//测试数据的通过数量
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

//测试null类型解析
static void test_parse_null()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " null "));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

//测试true类型
static void test_parse_true()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " true "));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

//测试false类型
static void test_parse_false()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " false "));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

//测试期待值的情况，这种情况都是只有空白符的情况
static void test_parse_expect_value()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));

	EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));
}

//测试不合理的输入
static void test_parse_invalid_value()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));

	EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, " t  r"));
	EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));
}

//测试不止是是单种类型
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
//总测试JSON解析器
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