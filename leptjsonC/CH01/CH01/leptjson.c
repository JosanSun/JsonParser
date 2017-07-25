#include "leptjson.h"
#include <assert.h>      //assert()
#include <stdlib.h>      //NULL

//ֻ�ṩ����API,����������staticʵ�ֿ�ķ�װ

//�����ַ���c�����ַ���ch�������ַ�����ƶ�һλ
#define EXPECT(c, ch) do{ assert(*c->json==(ch)); \
	++c->json;}while(0)

//Ҫ�������������ı�����
typedef struct
{
	const char* json;
}lept_context;

//���˿հ׷� ws = *(%x20 / %x09 / %x0A / %x0D)
static void lept_filter_whitespace(lept_context* c)
{
	const char* p = c->json;
	//������ò�Ҫ��<ctype.h>��isspace()
	while(' ' == *p || '\t' == *p || '\n' == *p || '\r' == *p)
	{
		++p;
	}
	c->json = p;
}

//����null����
static int lept_parse_null(lept_context* c, lept_value* v)
{
	//�ڴ�����'n'��ͬʱ++c->json
	EXPECT(c, 'n');
	//�����У��������Ϸ����ַ�������LEPT_PARSE_INVALID_VALUE
	if('u' != c->json[0] || 'l' != c->json[1] || 'l' != c->json[2])
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 3;
	v->type = LEPT_NULL;
	return LEPT_PARSE_OK;
}

//����true����
static int lept_parse_true(lept_context* c, lept_value* v)
{
	//�ڴ�����'t'��ͬʱ++c->json
	EXPECT(c, 't');
	//�����У��������Ϸ����ַ�������LEPT_PARSE_INVALID_VALUE
	if('r' != c->json[0] || 'u' != c->json[1] || 'e' != c->json[2])
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 3;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}

//����false����
static int lept_parse_false(lept_context* c, lept_value* v)
{
	//�ڴ�����'t'��ͬʱ++c->json
	EXPECT(c, 'f');
	//�����У��������Ϸ����ַ�������LEPT_PARSE_INVALID_VALUE
	if('a' != c->json[0] || 'l' != c->json[1] || 's' != c->json[2] || 'e' != c->json[3])
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 4;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
}

//�������ַ���ȷ������json���������ͷ���
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json)
	{
	case 'n':
		//���Խ���null
		return lept_parse_null(c, v);
	case 't':
		//���Խ���true
		return lept_parse_true(c, v);
	case 'f':
		//���Խ���false
		return lept_parse_false(c, v);
	case '\0':
		return LEPT_PARSE_EXPECT_VALUE;
	default:
		//���Ϸ����ַ�
		return LEPT_PARSE_INVALID_VALUE;
	}
}

//����json�����ı�
int lept_parse(lept_value* v, const char* json)
{
	lept_context cntxt;
	int ret;
	//����v��Ϊ��
	assert(NULL != v);
	//��ʼ��������c
	cntxt.json = json;
	//������������ǰ��Ŀհ׷�
	lept_filter_whitespace(&cntxt);
	//����cntxt��Ϣ�������������ret
	ret = lept_parse_value(&cntxt, v);
	if(LEPT_PARSE_OK == ret)
	{
		//�������˿հ׷�
		lept_filter_whitespace(&cntxt);
		//���cntxt�ı���Ȼ���ڴ��������ı�
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

//����JSON������
lept_type lept_get_type(const lept_value* v)
{
	assert(NULL != v);
	return v->type;
}