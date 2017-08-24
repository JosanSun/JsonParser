#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "leptjson.h"
#include <assert.h>    //assert()
#include <stdlib.h>    //NULL strtod()
#include <errno.h>     //errno, ERANGE, malloc(), realloc(), free()
#include <ctype.h>     //isdigit()
#include <math.h>      //HUGE_VAL
#include <string.h>    //memcpy()

//�ṩ12��API,����������staticʵ�ֿ�ķ�װ

//-----------------�궨�岿��----------------
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif
//�����ַ���c�����ַ���ch�������ַ�����ƶ�һλ
#define EXPECT(c, ch)\
	do {\
		assert(*c->json==(ch));\
		++c->json;\
	}while(0)
//�ж�ch�Ƿ���1-9֮��
#define ISDIGIT1TO9(ch) ((ch) <= '9' && '1' <= (ch))
#define ISHEX(ch)		( ((ch) <= '9' && '0' <= (ch)) || \
							((toupper(ch) <= 'F' && 'A' <= toupper(ch) )))
//��JSON������c��ջ��������ַ�ch
#define PUTC(c, ch) \
	do {\
		char* tem = (char*) lept_context_push(c, sizeof(char));\
		*tem = (ch);\
	}while(0)

//Ҫ�������������ı�����
//Ϊ�˴洢���������ַ���������һ��stack,
//����topָ��ջ��Ԫ�ص���һ��λ�ã�sz��ʾջ�Ĵ�С
//ջ��Ϊtop==0, ջ��Ϊtop == size
typedef struct
{
	const char* json;
	char* stack;
	size_t size, top;
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
	//-----------------------ע��ֱ����strtod()�Ὣ��಻�Ϸ�����ֵ�Ͷ���ȷת��
	////end��������strtod()�Ƿ�����ɹ�,���Գɹ�������ȥ
	/*char* end;
	v->num = strtod(c->json, &end);
	if(c->json == end)
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json = end;*/
	//-------------------------
	v->uni.num = strtod(c->json, NULL);
	//��������������������errno��ΪERANGE�����ǳ�����Χ��ǣ�
	//�����HUGE_VAL == v->num || -HUGE_VAL == v->num����ȷ�������������inf��
	//���Ǹ�������� -inf
	if(errno == ERANGE && (HUGE_VAL == v->uni.num || -HUGE_VAL == v->uni.num))
	{
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	c->json = p;
	return LEPT_PARSE_OK;
}

//��������c��ջ������sz�����ջ������ÿ������1.5���ռ䣬ֱ������Ҫ��
//����ֵ��֮ǰջ��ջ������һ��Ԫ��
//c->stackָ��û�䣬
//c->sz��ջ����ʱ�򣬻����ӣ�������ʱ�򣬲��䡣
//c->top�����ı䡣
static void* lept_context_push(lept_context* c, size_t sz)
{
	void* ret;
	assert(0 < sz);
	if(c->size <= c->top + sz)
	{
		if(0 == c->size)
		{
			c->size = LEPT_PARSE_STACK_INIT_SIZE;
		}
		while(c->size <= c->top + sz)
		{
			//�ȼ���c->size*=1.5;
			c->size += c->size >> 1;
		}
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += sz;
	return ret;
}

//�������ܣ�ɾ��sz��ջԪ��
//ջ��Ԫ�ظ������ڴ�ɾ���ĸ���
//����ɾ��֮���ջ��Ԫ�ص���һ��λ��
static void* lept_context_pop(lept_context* c, size_t sz)
{
	assert(sz <= c->top);
	c->top -= sz;
	return c->stack + c->top;
}

//����/uXXXX�ġ�XXXX�����֣�����ֵ�浽*u����
static const char* lept_parse_hex4(const char* p, unsigned int* u)
{
	unsigned int sum = 0;
	int i = 0;
	for(; i < 4 && ISHEX(p[i]); ++i)
	{
		sum <<= 4;
		if(isdigit(p[i]))
		{
			sum += p[i] - '0';
		}
		else
		{
			sum += toupper(p[i]) - 'A' + 10;
		}
	}
	*u = sum;
	return (i < 4) ? NULL : p + i;
}

//��Unicode�ַ�����UTF8�ı��뷽�����б���
static void lept_encode_utf8(lept_context* c, unsigned int uInt)
{
	if(uInt < 0x0080)
	{
		PUTC(c, uInt & 0xFF);
	}
	else if(uInt < 0x0800)
	{
		PUTC(c, 0xC0 | ((uInt >> 6) & 0xFF));
		PUTC(c, 0x80 | (uInt & 0x3F));
	}
	else if(uInt < 0x10000)
	{
		PUTC(c, 0xE0 | ((uInt >> 12) & 0xFF));
		PUTC(c, 0x80 | ((uInt >> 6) & 0x3F));
		PUTC(c, 0x80 | (uInt & 0x3F));
	}
	else
	{
		assert(uInt <= 0x10FFFF);
		PUTC(c, 0xF0 | ((uInt >> 18) & 0xFF));
		PUTC(c, 0x80 | ((uInt >> 12) & 0x3F));
		PUTC(c, 0x80 | ((uInt >> 6) & 0x3F));
		PUTC(c, 0x80 | (uInt & 0x3F));
	}
}

#define STRING_ERROR(ret) \
	do{\
		c->top = head;\
		return ret;\
	}while(0)
//����string����Ԫ��
static int lept_parse_string(lept_context* c, lept_value* v)
{
	size_t head = c->top, len;
	unsigned int uInt = 0, uInt2 = 0;
	const char* p;
	EXPECT(c, '\"');
	p = c->json;
	while(1)
	{
		char ch = *p++;
		switch(ch)
		{
		case '\"':
			//���ս�β�ַ�
			len = c->top - head;
			const char* topLink = (const char*)lept_context_pop(c, len);
			lept_set_string(v, topLink, len);
			c->json = p;
			return LEPT_PARSE_OK;
		case '\\':
			//����ת���ַ�
			switch(*p++)
			{
			case '\"': PUTC(c, '\"'); break;
			case '\\': PUTC(c, '\\'); break;
			case '/': PUTC(c, '/'); break;
			case 'b': PUTC(c, '\b'); break;
			case 'f': PUTC(c, '\f'); break;
			case 'n': PUTC(c, '\n'); break;
			case 'r': PUTC(c, '\r'); break;
			case 't': PUTC(c, '\t'); break;
			case 'u':
				//����'\u'�����׼������Unicode�ַ�
				p = lept_parse_hex4(p, &uInt);
				if(NULL == p)
				{
					STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
				}
				//����uInt��[0xD800, 0xDBFF]��Χ���ַ�����Ϊ�Ǹߴ������д���ԵĽ���
				//�ߴ����Χ[0xD800, 0xDBFF]���ʹ����Χ[0xDC00, 0xDFFF]
				if(uInt <= 0xDBFF && 0xD800 <= uInt)
				{
					if(*p != '\\' || *(p + 1) != 'u')
					{
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					}
					p += 2;
					p = lept_parse_hex4(p, &uInt2);
					if(NULL == p)
					{
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
					}
					if(uInt2 < 0xDC00 || 0xDFFF < uInt2)
					{
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					}
					uInt = (((uInt - 0xD800) << 10) | (uInt2 - 0xDC00)) + 0x10000;
				}
				lept_encode_utf8(c, uInt);
				break;
			//�Ƿ���ת���ַ�
			default:
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		case '\0':
			//ȱ�ٽ�β
			STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
		default:
			//����ASCIIֵС��0x20����Ϊ�ǷǷ��ַ�
			if((unsigned char)ch < 0x20)
			{
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
			}
			PUTC(c, ch);
		}
	}
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
		//JSON�ַ���Ϊ����Ϊ��
		return LEPT_PARSE_EXPECT_VALUE;
	case '\"':
		//���Խ���string����
		return lept_parse_string(c, v);
	default:
		//���Ϸ����ַ�---��ʱ�������Ϊ����number���ͣ���Ȼ�е����⣩
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
	//��ʼ��������c�е�json�ı�ָ�룬ջָ�룬ջ�ռ��С���Ѷ�
	cntxt.json = json;
	cntxt.stack = NULL;
	cntxt.size = cntxt.top = 0;
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
	assert(0 == cntxt.top);
	free(cntxt.stack);
	return ret;
}

//����JSON������
lept_type lept_get_type(const lept_value* v)
{
	assert(NULL != v);
	return v->type;
}

//�ͷ�JSONԪ����ռ�õĿռ䣬����Ԫ����������ΪLEPT_ERROR
void lept_free(lept_value* v)
{
	assert(NULL != v);
	if(LEPT_STRING == v->type)
	{
		free(v->uni.str.s);
	}
	v->type = LEPT_ERROR;
}

//----------------��ȡJSONԪ��ֵ������JSONԪ��ֵ�Ľӿ�---------
int lept_get_null(const lept_value* v)
{
	assert(NULL != v && (LEPT_NULL == v->type));
	return v->type;
}

void lept_set_null(lept_value* v)
{
	lept_free(v);
	v->type = LEPT_NULL;
}

int lept_get_boolean(const lept_value* v)
{
	assert(NULL != v && (LEPT_TRUE == v->type || LEPT_FALSE == v->type));
	return v->type;
}

void lept_set_boolean(lept_value* v, int b)
{
	lept_free(v);
	v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value* v)
{
	assert(NULL != v&&LEPT_NUMBER == v->type);
	return v->uni.num;
}

void lept_set_number(lept_value* v, double num)
{
	//�˴�v����Ϊ�ǿա����գ�lept_free(v)�ᱨ��
	lept_free(v);
	v->uni.num = num;
	v->type = LEPT_NUMBER;
}

const char* lept_get_string(const lept_value* v)
{
	assert(NULL != v&& LEPT_STRING == v->type);
	return v->uni.str.s;
}

size_t lept_get_string_length(const lept_value* v)
{
	assert(NULL != v && LEPT_STRING == v->type);
	return v->uni.str.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len)
{
	assert(s != NULL || 0 == len);
	lept_free(v);
	v->uni.str.s = (char*)malloc(len + 1);
	memcpy(v->uni.str.s, s, len);
	v->uni.str.s[len] = '\0';
	v->uni.str.len = len;
	v->type = LEPT_STRING;
}