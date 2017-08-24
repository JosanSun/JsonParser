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

//提供12个API,其他函数用static实现库的封装

//-----------------宏定义部分----------------
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif
//断言字符串c的首字符是ch，并将字符向后移动一位
#define EXPECT(c, ch)\
	do {\
		assert(*c->json==(ch));\
		++c->json;\
	}while(0)
//判断ch是否在1-9之内
#define ISDIGIT1TO9(ch) ((ch) <= '9' && '1' <= (ch))
#define ISHEX(ch)		( ((ch) <= '9' && '0' <= (ch)) || \
							((toupper(ch) <= 'F' && 'A' <= toupper(ch) )))
//在JSON上下文c的栈顶，添加字符ch
#define PUTC(c, ch) \
	do {\
		char* tem = (char*) lept_context_push(c, sizeof(char));\
		*tem = (ch);\
	}while(0)

//要解析的上下文文本类型
//为了存储解析到的字符串，设置一个stack,
//其中top指向栈顶元素的下一个位置，sz表示栈的大小
//栈空为top==0, 栈满为top == size
typedef struct
{
	const char* json;
	char* stack;
	size_t size, top;
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
	//-----------------------注意直接用strtod()会将许多不合法的数值型都正确转换
	////end用来测试strtod()是否解析成功,测试成功可以削去
	/*char* end;
	v->num = strtod(c->json, &end);
	if(c->json == end)
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json = end;*/
	//-------------------------
	v->uni.num = strtod(c->json, NULL);
	//上溢出和下溢出，都会是errno变为ERANGE（就是超出范围标记）
	//后面的HUGE_VAL == v->num || -HUGE_VAL == v->num，是确定是正数上溢出inf，
	//还是负数上溢出 -inf
	if(errno == ERANGE && (HUGE_VAL == v->uni.num || -HUGE_VAL == v->uni.num))
	{
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	c->json = p;
	return LEPT_PARSE_OK;
}

//给上下文c的栈顶增加sz，如果栈满，则每次申请1.5倍空间，直到满足要求
//返回值是之前栈的栈顶的下一个元素
//c->stack指针没变，
//c->sz在栈满的时候，会增加；不满的时候，不变。
//c->top发生改变。
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
			//等价于c->size*=1.5;
			c->size += c->size >> 1;
		}
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += sz;
	return ret;
}

//函数功能：删除sz个栈元素
//栈的元素个数大于待删除的个数
//返回删除之后的栈顶元素的下一个位置
static void* lept_context_pop(lept_context* c, size_t sz)
{
	assert(sz <= c->top);
	c->top -= sz;
	return c->stack + c->top;
}

//解析/uXXXX的“XXXX”部分，并将值存到*u里面
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

//对Unicode字符集用UTF8的编码方案进行编码
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
//解析string数据元素
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
			//接收结尾字符
			len = c->top - head;
			const char* topLink = (const char*)lept_context_pop(c, len);
			lept_set_string(v, topLink, len);
			c->json = p;
			return LEPT_PARSE_OK;
		case '\\':
			//插入转义字符
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
				//出现'\u'情况，准备解析Unicode字符
				p = lept_parse_hex4(p, &uInt);
				if(NULL == p)
				{
					STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
				}
				//对于uInt在[0xD800, 0xDBFF]范围的字符，认为是高代理，进行代理对的解析
				//高代理项范围[0xD800, 0xDBFF]；低代理项范围[0xDC00, 0xDFFF]
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
			//非法的转义字符
			default:
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		case '\0':
			//缺少结尾
			STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
		default:
			//对于ASCII值小于0x20，认为是非法字符
			if((unsigned char)ch < 0x20)
			{
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
			}
			PUTC(c, ch);
		}
	}
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
		//JSON字符串为本身为空
		return LEPT_PARSE_EXPECT_VALUE;
	case '\"':
		//尝试解析string类型
		return lept_parse_string(c, v);
	default:
		//不合法的字符---暂时将这里改为解析number类型（虽然有点问题）
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
	//初始化上下文c中的json文本指针，栈指针，栈空间大小，堆顶
	cntxt.json = json;
	cntxt.stack = NULL;
	cntxt.size = cntxt.top = 0;
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
	assert(0 == cntxt.top);
	free(cntxt.stack);
	return ret;
}

//返回JSON的类型
lept_type lept_get_type(const lept_value* v)
{
	assert(NULL != v);
	return v->type;
}

//释放JSON元素所占用的空间，并将元素类型设置为LEPT_ERROR
void lept_free(lept_value* v)
{
	assert(NULL != v);
	if(LEPT_STRING == v->type)
	{
		free(v->uni.str.s);
	}
	v->type = LEPT_ERROR;
}

//----------------获取JSON元素值和设置JSON元素值的接口---------
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
	//此处v必须为非空。若空，lept_free(v)会报错。
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