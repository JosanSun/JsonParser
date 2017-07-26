#include "leptjson.h"
#include <stdio.h>

//total number of previous test: 20
//total number of current test: 142

//CH02作为CH02新增的测试用例的开关，
//1表示加入这些新的测试用例，0表示关闭；
//这样很容易比较与之前一章的测试用例不同点
#define CH02 1

//主函数的返回值，0为正常返回，非0存在错误
static int main_ret = 0;
//测试数据的数量
static int test_cnt = 0;
//测试数据的通过数量
static int test_pass = 0;

//------------------CH02所用到的所有宏定义------------
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

//重构代码，将原来的测试简化 TEST_LITERAL可以用来测试null, true, false
//它们的共同点都是解析成功，即返回LEPT_PARSE_OK;但是解析出来的元素类型存在不同
#define TEST_LITERAL(init_type, json)\
	do {\
		lept_value v;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(init_type, lept_get_type(&v));\
	}while(0)
//TEST_NUMBER用来测试数值型类型，CH02新增部分
#define TEST_NUMBER(expt, json)\
	do {\
		lept_value v;\
        /* 测试是否解析成功 */                        \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
		/* 测试是否解析出正确数值类型 */                        \
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		/* 测试是否解析出正确数值 */                        \
		EXPECT_EQ_DOUBLE(expt, lept_get_number(&v));\
	}while(0)
//由于TEST_ERROR与TEST_LITERAL基本一致，只是第一个参数有所不同
//错误解析后的JSON对象的所有类型都是LEPT_ERROR,但是解析错误的原因却有多种可能
#define TEST_ERROR(errRetType, json)\
	do {\
		lept_value v;\
		EXPECT_EQ_INT(errRetType, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_ERROR, lept_get_type(&v));\
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
#endif

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
#if CH02
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
#endif
}

//测试不止是是单种类型
static void test_parse_root_not_singular()
{
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "truex ");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "false  null x");
#if CH02
	//0之后，应该是小数点或者没有值
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif
}

#if CH02
//测试新的返回错误，解析的数值过大
static void test_parse_number_too_big()
{
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}
#endif

//测试正确解析出类型的JSON元素
static void test_parse_ok()
{
	//测试null, 以及boolean
	test_parse_literal();
#if CH02
	//测试数值
	test_parse_number();
#endif
}

//总测试JSON解析器
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