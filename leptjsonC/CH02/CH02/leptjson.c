#include "leptjson.h"
#include <assert.h>      //assert()
#include <stdlib.h>      //NULL
#include <errno.h>       //errno, ERANGE
#include <ctype.h>       //isdigit()
#include <math.h>        //HUGE_VAL

//提供3个API,其他函数用static实现库的封装

//断言字符串c的首字符是ch，并将字符向后移动一位
#define EXPECT(c, ch)\
	do {\
		assert(*c->json==(ch));\
		++c->json;\
	}while(0)
#define ISDIGIT1TO9(ch) ((ch) <= '9' && '1' <= (ch))

//要解析的上下文文本类型
typedef struct
{
	const char* json;
}lept_context;

//过滤空白符 ws = *(%x20 / %x09 / %x0A / %x0D)
static void lept_filter_whitespace(lept_context* c)
{
	const char* p = c->json;
	//这里最好不要用<ctype.h>的isspace()
	while(' ' == *p || '\t' == *p || '\n' == *p || '\r' == *p)
	{
		++p;
	}
	c->json = p;
}

//解析null, true, false类型 c为json文本，v为解析之后json元素，
//literal为预期解析的字符串， type为解析成功之后的元素类型
static int lept_parse_literal(lept_context* c, lept_value* v, 
						const char* literal, lept_type type)
{
	size_t i;
	//期待出现literal[0]，同时++c->json
	EXPECT(c, literal[0]);
	//解析中，遇到不合法的字符，返回LEPT_PARSE_INVALID_VALUE
	for(i = 0; literal[i + 1]; ++i)
	{
		if(c->json[i] != literal[i + 1])
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
	}
	c->json += i;
	v->type = type;
	return LEPT_PARSE_OK;
}

//解析number类型
static int lept_parse_number(lept_context* c, lept_value* v)
{
	const char* p = c->json;
	//----处理'-'
	if('-' == *p)
	{
		++p;
	}
	//----处理整数部分
	//如果只有0
	if('0' == *p)
	{
		++p;
	}
	else
	{
		//非0数值的第一个数字必须为1-9
		if(!ISDIGIT1TO9(*p))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
		//处理其他数字
		for(++p; isdigit(*p); ++p);
	}
	//----处理小数部分
	if('.' == *p)
	{
		++p;
		//小数点之后，必须至少有一个数字
		if(!isdigit(*p))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
		//处理其他数字
		for(++p; isdigit(*p); ++p);
	}
	//----处理指数部分
	if('e' == *p || 'E' == *p)
	{
		++p;
		if('+' == *p || '-' == *p)
		{
			++p;
		}
		//e/E除了'+','-'之后，至少要有一个数字
		if(!isdigit(*p))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
		//处理其他数字
		for(++p; isdigit(*p); ++p);
	}
	v->type = LEPT_NUMBER;
	errno = 0;
	v->num = strtod(c->json, NULL);
	//上溢出和下溢出，都会是errno变为ERANGE（就是超出范围标记）
	//后面的HUGE_VAL == v->num || -HUGE_VAL == v->num，是确定是正数上溢出inf，
	//还是负数上溢出 -inf
	if(errno == ERANGE && (HUGE_VAL == v->num || -HUGE_VAL == v->num))
	{
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	c->json = p;
	return LEPT_PARSE_OK;
}

//解析json的数据类型    ---依据首字符，确定解析json的数据类型方法
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json)
	{
	case 'n':
		//尝试解析null
		return lept_parse_literal(c, v, "null", LEPT_NULL);
	case 't':
		//尝试解析true
		return lept_parse_literal(c, v, "true", LEPT_TRUE);
	case 'f':
		//尝试解析false
		return lept_parse_literal(c, v, "false", LEPT_FALSE);
	case '\0':
		return LEPT_PARSE_EXPECT_VALUE;
	default:
		//不合法的字符---暂时将这里改为解析数字（虽然有点问题）
		return lept_parse_number(c, v);
	}
}

//解析json数据文本
int lept_parse(lept_value* v, const char* json)
{
	lept_context cntxt;
	int ret;
	//断言v不为空
	assert(NULL != v);
	//初始化上下文c
	cntxt.json = json;
	//过滤上下文中前面的空白符
	lept_filter_whitespace(&cntxt);
	//解析cntxt信息，解析结果返回ret
	ret = lept_parse_value(&cntxt, v);
	if(LEPT_PARSE_OK == ret)
	{
		//继续过滤空白符
		lept_filter_whitespace(&cntxt);
		//如果cntxt文本仍然存在待解析的文本
		if(*cntxt.json)
		{
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	if(LEPT_PARSE_OK != ret)
	{
		v->type = LEPT_ERROR;
	}
	return ret;
}

//返回JSON的类型
lept_type lept_get_type(const lept_value* v)
{
	assert(NULL != v);
	return v->type;
}

double lept_get_number(const lept_value* v)
{
	assert(NULL != v);
	return v->num;
}