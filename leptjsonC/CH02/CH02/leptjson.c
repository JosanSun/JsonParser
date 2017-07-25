#include "leptjson.h"
#include <assert.h>      //assert()
#include <stdlib.h>      //NULL
#include <errno.h>       //errno, ERANGE
#include <ctype.h>       //isdigit()
#include <math.h>        //HUGE_VAL

//�ṩ3��API,����������staticʵ�ֿ�ķ�װ

//�����ַ���c�����ַ���ch�������ַ�����ƶ�һλ
#define EXPECT(c, ch)\
	do {\
		assert(*c->json==(ch));\
		++c->json;\
	}while(0)
#define ISDIGIT1TO9(ch) ((ch) <= '9' && '1' <= (ch))

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

//����null, true, false���� cΪjson�ı���vΪ����֮��jsonԪ�أ�
//literalΪԤ�ڽ������ַ����� typeΪ�����ɹ�֮���Ԫ������
static int lept_parse_literal(lept_context* c, lept_value* v, 
						const char* literal, lept_type type)
{
	size_t i;
	//�ڴ�����literal[0]��ͬʱ++c->json
	EXPECT(c, literal[0]);
	//�����У��������Ϸ����ַ�������LEPT_PARSE_INVALID_VALUE
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

//����number����
static int lept_parse_number(lept_context* c, lept_value* v)
{
	const char* p = c->json;
	//----����'-'
	if('-' == *p)
	{
		++p;
	}
	//----������������
	//���ֻ��0
	if('0' == *p)
	{
		++p;
	}
	else
	{
		//��0��ֵ�ĵ�һ�����ֱ���Ϊ1-9
		if(!ISDIGIT1TO9(*p))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
		//������������
		for(++p; isdigit(*p); ++p);
	}
	//----����С������
	if('.' == *p)
	{
		++p;
		//С����֮�󣬱���������һ������
		if(!isdigit(*p))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
		//������������
		for(++p; isdigit(*p); ++p);
	}
	//----����ָ������
	if('e' == *p || 'E' == *p)
	{
		++p;
		if('+' == *p || '-' == *p)
		{
			++p;
		}
		//e/E����'+','-'֮������Ҫ��һ������
		if(!isdigit(*p))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
		//������������
		for(++p; isdigit(*p); ++p);
	}
	v->type = LEPT_NUMBER;
	errno = 0;
	v->num = strtod(c->json, NULL);
	//��������������������errno��ΪERANGE�����ǳ�����Χ��ǣ�
	//�����HUGE_VAL == v->num || -HUGE_VAL == v->num����ȷ�������������inf��
	//���Ǹ�������� -inf
	if(errno == ERANGE && (HUGE_VAL == v->num || -HUGE_VAL == v->num))
	{
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	c->json = p;
	return LEPT_PARSE_OK;
}

//����json����������    ---�������ַ���ȷ������json���������ͷ���
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json)
	{
	case 'n':
		//���Խ���null
		return lept_parse_literal(c, v, "null", LEPT_NULL);
	case 't':
		//���Խ���true
		return lept_parse_literal(c, v, "true", LEPT_TRUE);
	case 'f':
		//���Խ���false
		return lept_parse_literal(c, v, "false", LEPT_FALSE);
	case '\0':
		return LEPT_PARSE_EXPECT_VALUE;
	default:
		//���Ϸ����ַ�---��ʱ�������Ϊ�������֣���Ȼ�е����⣩
		return lept_parse_number(c, v);
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

double lept_get_number(const lept_value* v)
{
	assert(NULL != v);
	return v->num;
}