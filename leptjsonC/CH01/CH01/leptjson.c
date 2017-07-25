#include "leptjson.h"
#include <assert.h>      //assert()
#include <stdlib.h>      //NULL

//只提供两个API,其他函数用static实现库的封装

//断言字符串c的首字符是ch，并将字符向后移动一位
#define EXPECT(c, ch) do{ assert(*c->json==(ch)); \
	++c->json;}while(0)

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

//解析null类型
static int lept_parse_null(lept_context* c, lept_value* v)
{
	//期待出现'n'，同时++c->json
	EXPECT(c, 'n');
	//解析中，遇到不合法的字符，返回LEPT_PARSE_INVALID_VALUE
	if('u' != c->json[0] || 'l' != c->json[1] || 'l' != c->json[2])
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 3;
	v->type = LEPT_NULL;
	return LEPT_PARSE_OK;
}

//解析true类型
static int lept_parse_true(lept_context* c, lept_value* v)
{
	//期待出现't'，同时++c->json
	EXPECT(c, 't');
	//解析中，遇到不合法的字符，返回LEPT_PARSE_INVALID_VALUE
	if('r' != c->json[0] || 'u' != c->json[1] || 'e' != c->json[2])
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 3;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}

//解析false类型
static int lept_parse_false(lept_context* c, lept_value* v)
{
	//期待出现't'，同时++c->json
	EXPECT(c, 'f');
	//解析中，遇到不合法的字符，返回LEPT_PARSE_INVALID_VALUE
	if('a' != c->json[0] || 'l' != c->json[1] || 's' != c->json[2] || 'e' != c->json[3])
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 4;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
}

//依据首字符，确定解析json的数据类型方法
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json)
	{
	case 'n':
		//尝试解析null
		return lept_parse_null(c, v);
	case 't':
		//尝试解析true
		return lept_parse_true(c, v);
	case 'f':
		//尝试解析false
		return lept_parse_false(c, v);
	case '\0':
		return LEPT_PARSE_EXPECT_VALUE;
	default:
		//不合法的字符
		return LEPT_PARSE_INVALID_VALUE;
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