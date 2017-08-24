#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>    //NULL strtod()
#include <crtdbg.h>
#include "leptjson.h"
#include <assert.h>    //assert()
#include <stdlib.h>    
#include <errno.h>     //errno, ERANGE, malloc(), realloc(), free()
#include <ctype.h>     //isdigit()
#include <math.h>      //HUGE_VAL
#include <string.h>    //memcpy()
#include <stdio.h>     //sprintf()

//�ṩ14��API,����������staticʵ�ֿ�ķ�װ

//-----------------�궨�岿��----------------
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef LEPT_PARSE_STRINGIFY_INIT_SIZE
#define LEPT_PARSE_STRINGIFY_INIT_SIZE 256
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
//����len��С���ַ�
#define PUTS(c, s, len)   memcpy(lept_context_push(c, len), s, len)

//Ҫ�������������ı�����
//Ϊ�˴洢���������ַ���������һ��stack,
//����topָ��ջ��Ԫ�ص���һ��λ�ã�size��ʾջ�Ŀռ��С�� size��top�ĵ�λ�����ֽ�
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

//�ع�����string����Ԫ��
//����ԭ����string�ַ�������JSONstring��һ���֣����漰lept_value* v�ĸ�ֵ

static int lept_parse_string_raw(lept_context* c, char** s, size_t* l)
{
	//ע��˴�������head����Ϊhead���ܲ�Ϊ0�������������ʱ��������c��ջ�����Ѿ�����Ԫ�أ����head���ܲ�Ϊ0��
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
			*l = len;
			*s = (char*)malloc(len + 1);
			memcpy((void*)*s, topLink, len);
			(*s)[len] = '\0';
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
			//Question:����������޼����ַ���
			PUTC(c, ch);
		}
	}
}

//����string����Ԫ��
static int lept_parse_string(lept_context* c, lept_value* v)
{
	int ret;
	char* s;
	size_t len;

	ret = lept_parse_string_raw(c, &s, &len);
	if(LEPT_PARSE_OK == ret)
	{
		lept_set_string(v, s, len);
		free(s);
	}
	return ret;
}

//ǰ��������Ϊ������lept_parse_array����ǰ����lept_parse_value
static int lept_parse_value(lept_context* c, lept_value* v);

//����array����Ԫ��
//array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
static int lept_parse_array(lept_context* c, lept_value* v)
{
	size_t size = 0;
	size_t i;
	int ret;
	EXPECT(c, '[');
	//���˿հ׷�
	lept_filter_whitespace(c);
	if(']' == *c->json)
	{
		++c->json;
		v->type = LEPT_ARRAY;
		v->uni.arr.size = 0;
		v->uni.arr.e = NULL;
		return LEPT_PARSE_OK;
	}
	while(1)
	{
		lept_value val;
		lept_init(&val);
		ret = lept_parse_value(c, &val);
		if(LEPT_PARSE_OK != ret)
		{
			////Question : sz����0���ᱨ��
			//lept_context_pop(c, size * sizeof(lept_value));
			//return ret;
			break;
		}
		//����sizeof(lept_value)��С�Ŀռ�������Ѿ�����������arr�е�Ԫ��
		memcpy(lept_context_push(c, sizeof(lept_value)), &val, sizeof(lept_value));
		//����һ������Ԫ��
		++size;
		lept_filter_whitespace(c);
		if(',' == *c->json)
		{
			++c->json;
			lept_filter_whitespace(c);
		}
		else if(']' == *c->json)
		{
			++c->json;
			v->type = LEPT_ARRAY;
			v->uni.arr.size = size;
			//size��ʾԪ�صĸ����� sz��ʾ����Ԫ����ռ�ռ�Ĵ�С
			int sz = size * sizeof(lept_value);
			//��JSONԪ��v����������ָ�����sz��С�ռ䣬�������size��lept_valueԪ�أ���ʱδ��ֵ
			v->uni.arr.e = (lept_value*)malloc(sz);
			//��JSON�����ĵ�c��ջ��top����sz,ͬʱ��c��ջ��ֵ��v->uni.arr.eָ��Ŀռ�
			memcpy((void*)v->uni.arr.e, lept_context_pop(c, sz), sz);
			return LEPT_PARSE_OK;
		}
		else
		{
			//ȱ�ٶ����Լ��ҷ��������
			//���磺'\0', '{'�����
			ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	//NOTE:����size����Ϊ0
	for(i = 0; i < size; ++i)
	{
		lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
	}
	return ret;
}

//�ͷų�Ա����
static void lept_free_member(lept_member* m);
//����object����Ԫ��
static int lept_parse_object(lept_context* c, lept_value* v)
{
	size_t i, size = 0;
	size_t sz;
	lept_member member;
	int ret;
	EXPECT(c, '{');
	lept_filter_whitespace(c);
	//���1��������'}', ��Ϊ�ն���ֱ�ӷ���, size=0
	if('}' == *c->json)
	{
		++c->json;
		v->type = LEPT_OBJECT;
		v->uni.obj.m = NULL;
		v->uni.obj.size = 0;
		return LEPT_PARSE_OK;
	}
	member.key = NULL;
	while(1)
	{
		lept_init(&member.val);
		//���2�����Ƚ����ؼ��֣������ڴ�'"'�����û�г���'"',��ret = LEPT_PARSE_MISS_KEY;
		if('\"' != *c->json)
		{
			ret = LEPT_PARSE_MISS_KEY;
			break;
		}
		ret = lept_parse_string_raw(c, &member.key, &member.keyLen);
		if(LEPT_PARSE_OK != ret)
		{
			break;
		}
		lept_filter_whitespace(c);
		if(':' != *c->json)
		{
			ret = LEPT_PARSE_MISS_COLON;
			break;
		}
		++c->json;
		lept_filter_whitespace(c);
		ret = lept_parse_value(c, &member.val);
		if(LEPT_PARSE_OK != ret)
		{
			break;
		}
		memcpy(lept_context_push(c, sizeof(lept_member)), &member, sizeof(lept_member));
		//���ˣ�һ������Ľ����ɹ���size+1
		++size;
		//member.key��ʼ��
		member.key = NULL;
		lept_filter_whitespace(c);
		if(',' == *c->json)
		{
			++c->json;
			lept_filter_whitespace(c);
		}
		else if('}' == *c->json)
		{
			++c->json;
			v->type = LEPT_OBJECT;
			v->uni.obj.size = size;
			sz = size * sizeof(lept_member);
			v->uni.obj.m = (lept_member*)malloc(sz);
			memcpy((void *)v->uni.obj.m, lept_context_pop(c, sz), sz);
			return LEPT_PARSE_OK;
		}
		else
		{
			ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
	}
	//NOTE:��������,�ͷ�key
	free(member.key);
	for(i = 0; i < size; ++i)
	{
		lept_free_member((lept_member*)lept_context_pop(c, sizeof(lept_member)));
	}
	return ret;
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
	case '[':
		//���Խ���array����
		return lept_parse_array(c, v);
	case '{':
		//���Խ���object����
		return lept_parse_object(c, v);
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
	//��ֹ�ڴ�й©
	assert(0 == cntxt.top);
	free(cntxt.stack);
	return ret;
}

//δ�����Ż��������ַ���
#if 0
static void lept_stringify_string(lept_context* c, const char* s, size_t len)
{
	size_t i;
	assert(NULL != s);
	PUTC(c, '\"');
	for(i = 0; i < len; ++i)
	{
		unsigned char ch = (unsigned char)s[i];
		switch(ch)
		{
		case '\"': case '\\': case '\b': 
		case '\f': case '\n': case '\r': 
		case '\t': 
			PUTC(c, '\\');
			PUTC(c, ch);
			break;
		default:
			if(ch < 0x20)
			{
				char buffer[7];
				sprintf(buffer, "\\u%04X", ch);
				//"\\u"�ַ���2���ַ� + XXXX  4��16�����ַ�
				PUTS(c, buffer, 6);
			}
			else
			{
				PUTC(c, s[i]);
			}
		}
	}
	/*if(len)
	{
		//PUTSʱ������ȷ��len��Ϊ0��
		//ERROR: ����ֱ�����Ԫ�ز�����
		PUTS(c, s, len);
	}*/
	PUTC(c, '\"');
}
#endif

static void lept_stringify_string(lept_context* c, const char* s, size_t len)
{
	static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	size_t i, size;
	char* head, *p;
	assert(NULL != s);
	size = len * 6 + 2;
	p = head = lept_context_push(c, size);
	*p++ = '\"';
	for(i = 0; i < len; ++i)
	{
		unsigned char ch = (unsigned char)s[i];
		switch(ch)
		{
		case '\"': 
			*p++ = '\\'; 
			*p++ = '\"'; 
			break;
		case '\\': 
			*p++ = '\\'; 
			*p++ = '\\'; 
			break;
		case '\b': 
			*p++ = '\\'; 
			//ע�ⲻ��*p++='\b';
			*p++ = 'b';  
			break;
		case '\f': 
			*p++ = '\\'; 
			*p++ = 'f';  
			break;
		case '\n': 
			*p++ = '\\'; 
			*p++ = 'n';  
			break;
		case '\r': 
			*p++ = '\\'; 
			*p++ = 'r';  
			break;
		case '\t': 
			*p++ = '\\'; 
			*p++ = 't';  
			break;
		default:
			if(ch < 0x20)
			{
				*p++ = '\\';
				*p++ = 'u';
				*p++ = '0';
				*p++ = '0';
				*p++ = hex_digits[ch >> 4];
				*p++ = hex_digits[ch & 15];
			}
			else
			{
				*p++ = s[i];
			}
		}
	}
	*p++ = '\"';
	//p-head�õ����������ַ����ĳ��ȣ������������ŵģ�
	//c->top����Ϊջ��λ��ָ����һ���ַ���Ԫ�ص�λ�ã�ջԪ�ؾ������ɵ��ַ���
	c->top -= size - (p - head);
}

static void lept_stringify_value(lept_context* c, const lept_value* v);

static void lept_stringify_array(lept_context* c, const lept_value* e, size_t size)
{
	size_t i = 0;
	PUTC(c, '[');
	for(; i < size; ++i)
	{
		//��ȷ�Ӷ��ŵ���һ�ַ���
		/*if(0 < i)
		{
			PUTC(c, ',');
		}*/
		lept_stringify_value(c, e + i);
		if(size - 1 != i)
		{
			PUTC(c, ',');
		}
	}
	PUTC(c, ']');
}

static void lept_stringify_member(lept_context* c, const lept_member* m)
{
	lept_stringify_string(c, m->key, m->keyLen);
	PUTC(c, ':');
	lept_stringify_value(c, &m->val);
}

static void lept_stringify_object(lept_context* c, const lept_member* m, size_t size)
{
	size_t i = 0;
	PUTC(c, '{');
	for(; i < size; ++i)
	{
		lept_stringify_member(c, m + i);
		if(size - 1 != i)
		{
			PUTC(c, ',');
		}
	}
	PUTC(c, '}');
}

static void lept_stringify_value(lept_context* c, const lept_value* v)
{
	switch(v->type)
	{
	case LEPT_NULL:
		PUTS(c, "null", 4);
		break;
	case LEPT_TRUE:
		PUTS(c, "true", 4);
		break;
	case LEPT_FALSE:
		PUTS(c, "false", 5);
		break;
	case LEPT_NUMBER:
		c->top -= 32 - sprintf(lept_context_push(c, 32), "%.17g", v->uni.num);
		break;
	case LEPT_STRING:
		lept_stringify_string(c, v->uni.str.s, v->uni.str.len);
		break;
	case LEPT_ARRAY:
		lept_stringify_array(c, v->uni.arr.e, v->uni.arr.size);
		break;
	case LEPT_OBJECT:
		lept_stringify_object(c, v->uni.obj.m, v->uni.obj.size);
		break;
	default:
		assert(0 && "invalid type");
	}
}

char* lept_stringify(const lept_value* v, size_t* l)
{
	lept_context cntxt;
	assert(NULL != v);
	cntxt.stack = (char*)malloc(LEPT_PARSE_STRINGIFY_INIT_SIZE);
	cntxt.size = LEPT_PARSE_STRINGIFY_INIT_SIZE;
	cntxt.top = 0;
	lept_stringify_value(&cntxt, v);
	if(l)
	{
		*l = cntxt.top;
	}
	PUTC(&cntxt, '\0');
	return cntxt.stack;
}


//����JSON������
lept_type lept_get_type(const lept_value* v)
{
	assert(NULL != v);
	return v->type;
}

//�ͷų�Ա
static void lept_free_member(lept_member* m)
{
	assert(NULL != m);
	free(m->key);
	lept_free(&m->val);
}

//�ͷ�JSONԪ����ռ�õĿռ䣬����Ԫ����������ΪLEPT_ERROR
void lept_free(lept_value* v)
{
	assert(NULL != v);
	size_t i;
	switch(v->type)
	{
		//���v��������string��ֱ���ͷſռ�
	case LEPT_STRING:
		free(v->uni.str.s);
		break;
		//���v��������array���ݹ��ͷ���������Ԫ�صĿռ�
	case LEPT_ARRAY:
		for(i = 0; i < v->uni.arr.size; ++i)
		{
			lept_free(v->uni.arr.e + i);
		}
		free(v->uni.arr.e);
		break;
	case LEPT_OBJECT:
		for(i = 0; i < v->uni.obj.size; ++i)
		{
			lept_free_member(v->uni.obj.m + i);
		}
		free(v->uni.arr.e);
	default:
		break;
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

lept_value* lept_get_array_element(const lept_value* v, size_t index)
{
	assert(NULL != v&&LEPT_ARRAY == v->type&&index < v->uni.arr.size);
	return &v->uni.arr.e[index];
}

size_t lept_get_array_size(const lept_value* v)
{
	assert(NULL != v &&LEPT_ARRAY == v->type);
	return v->uni.arr.size;
}

const char* lept_get_object_key(const lept_value* v, size_t index)
{
	assert(NULL != v&&LEPT_OBJECT == v->type);
	assert(index < v->uni.obj.size);
	return v->uni.obj.m[index].key;
}

size_t lept_get_object_key_length(const lept_value* v, size_t index)
{
	assert(NULL != v&&LEPT_OBJECT == v->type);
	assert(index < v->uni.obj.size);
	return v->uni.obj.m[index].keyLen;
}

lept_value* lept_get_object_value(const lept_value* v, size_t index)
{
	assert(NULL != v&&LEPT_OBJECT == v->type);
	assert(index < v->uni.obj.size);
	return &v->uni.obj.m[index].val;
}

size_t lept_get_object_size(const lept_value* v)
{
	assert(NULL != v&&LEPT_OBJECT == v->type);
	return v->uni.obj.size;
}