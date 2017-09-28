/*
 * ---------------------------------------------------
 *  Copyright (c) 2017 josan All rights reserved.
 * ---------------------------------------------------
 *
 *               创建者： Josan
 *             创建时间： 2017/9/28 10:36:52
 */
#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>    //NULL strtod(), malloc(), realloc(), free()
#include <crtdbg.h>
#include <cassert>    //assert()
#include <cstdlib>    
#include <cerrno>     //errno, ERANGE
#include <cctype>     //isdigit()
#include <cmath>      //HUGE_VAL
#include <cstring>    //memcpy()
#include <sstream>     //sprintf()
 //提供14个API,其他函数用static实现库的封装
#include "json.h"
 //-----------------宏定义部分----------------
#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef JSON_PARSE_STRINGIFY_INIT_SIZE
#define JSON_PARSE_STRINGIFY_INIT_SIZE 256
#endif

namespace SEU_josan_JSON
{

	//判断ch是否在1-9之内
	inline bool isDigit1to9(char ch)
	{
		return (ch) <= '9' && '1' <= (ch);
	}
	//判断ch是否是16进制数
	inline bool isHex(char ch)
	{
		return ((ch) <= '9' && '0' <= (ch)) ||
			((toupper(ch) <= 'F' && 'A' <= toupper(ch)));
	}
	//给上下文c的栈顶增加sz，如果栈满，则每次申请1.5倍空间，直到满足要求
	//返回值是之前栈的栈顶的下一个元素
	//c->stack指针没变，
	//c->sz在栈满的时候，会增加；不满的时候，不变。
	//c->top发生改变。
	void* json_context::json_context_push(size_t sz)
	{
		void* ret;
		assert(0 < sz);
		if(size <= top + sz)
		{
			if(0 == size)
			{
				size = JSON_PARSE_STACK_INIT_SIZE;
			}
			while(size <= top + sz)
			{
				//等价于c->size*=1.5;
				size += size >> 1;
			}
			stack = (char*)realloc(stack, size);
		}
		ret = stack + top;
		top += sz;
		return ret;
	}
	//过滤空白符 ws = *(%x20 / %x09 / %x0A / %x0D)
	void json_context::json_filter_whitespace()
	{
		const char* p = json;
		//这里最好不要用<ctype.h>的isspace()
		while(' ' == *p || '\t' == *p || '\n' == *p || '\r' == *p)
		{
			++p;
		}
		json = p;
	}

	//解析null, true, false类型 c为json文本，v为解析之后json元素，
	//literal为预期解析的字符串， type为解析成功之后的元素类型
	returnType json_context::json_parse_literal(json_value* v,
								  const char* literal, json_type type)
	{
		size_t i;
		//期待出现literal[0]，同时++c->json
		EXPECT(literal[0]);
		//解析中，遇到不合法的字符，返回JSON_PARSE_INVALID_VALUE
		for(i = 0; literal[i + 1]; ++i)
		{
			if(json[i] != literal[i + 1])
			{
				return JSON_PARSE_INVALID_VALUE;
			}
		}
		json += i;
		v->type = type;
		return JSON_PARSE_OK;
	}

	//解析number类型
	returnType json_context::json_parse_number(json_value* v)
	{

		const char* p = json;
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
			if(!isDigit1to9(*p))
			{
				return JSON_PARSE_INVALID_VALUE;
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
				return JSON_PARSE_INVALID_VALUE;
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
				return JSON_PARSE_INVALID_VALUE;
			}
			//处理其他数字
			for(++p; isdigit(*p); ++p);
		}
		v->type = JSON_NUMBER;
		errno = 0;
		//-----------------------注意直接用strtod()会将许多不合法的数值型都正确转换
		////end用来测试strtod()是否解析成功,测试成功可以削去
		/*char* end;
		v->num = strtod(c->json, &end);
		if(c->json == end)
		{
		return JSON_PARSE_INVALID_VALUE;
		}
		c->json = end;*/
		//-------------------------
		v->uni.num = strtod(json, NULL);
		//上溢出和下溢出，都会是errno变为ERANGE（就是超出范围标记）
		//后面的HUGE_VAL == v->num || -HUGE_VAL == v->num，是确定是正数上溢出inf，
		//还是负数上溢出 -inf
		if(errno == ERANGE && (HUGE_VAL == v->uni.num || -HUGE_VAL == v->uni.num))
		{
			return JSON_PARSE_NUMBER_TOO_BIG;
		}
		json = p;
		return JSON_PARSE_OK;
	}

	//解析/uXXXX的“XXXX”部分，并将值存到*u里面
	const char* json_context::json_parse_hex4(const char* p, unsigned int* u)
	{
		unsigned int sum = 0;
		int i = 0;
		for(; i < 4 && isHex(p[i]); ++i)
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
	void json_context::json_encode_utf8(unsigned int uInt)
	{
		if(uInt < 0x0080)
		{
			PUTC(uInt & 0xFF);
		}
		else if(uInt < 0x0800)
		{
			PUTC(0xC0 | ((uInt >> 6) & 0xFF));
			PUTC(0x80 | (uInt & 0x3F));
		}
		else if(uInt < 0x10000)
		{
			PUTC(0xE0 | ((uInt >> 12) & 0xFF));
			PUTC(0x80 | ((uInt >> 6) & 0x3F));
			PUTC(0x80 | (uInt & 0x3F));
		}
		else
		{
			assert(uInt <= 0x10FFFF);
			PUTC(0xF0 | ((uInt >> 18) & 0xFF));
			PUTC(0x80 | ((uInt >> 12) & 0x3F));
			PUTC(0x80 | ((uInt >> 6) & 0x3F));
			PUTC(0x80 | (uInt & 0x3F));
		}
	}
	
	//重构解析string数据元素
	//解析原生的string字符串，是JSONstring的一部分，不涉及json_value* v的赋值

	returnType json_context::json_parse_string_raw(char** s, size_t* l)
	{
		//注意此处必须有head，因为head可能不为0；详见解析数组时，上下文c的栈可能已经存在元素，因此head可能不为0；
		size_t head = top, len;
		unsigned int uInt = 0, uInt2 = 0;
		const char* p;
		EXPECT('\"');
		p = json;
		while(1)
		{
			char ch = *p++;
			switch(ch)
			{
			case '\"':
			{
				//接收结尾字符
				len = top - head;
				const char* topLink = reinterpret_cast<const char*>(json_context_pop(len));
				*l = len;
				*s = (char*)malloc(len + 1);
				memcpy((void*)*s, topLink, len);
				(*s)[len] = '\0';
				json = p;
				return JSON_PARSE_OK;
			}
			case '\\':
				//插入转义字符
				switch(*p++)
				{
				case '\"': PUTC('\"'); break;
				case '\\': PUTC('\\'); break;
				case '/': PUTC('/'); break;
				case 'b': PUTC('\b'); break;
				case 'f': PUTC('\f'); break;
				case 'n': PUTC('\n'); break;
				case 'r': PUTC('\r'); break;
				case 't': PUTC('\t'); break;
				case 'u':
					//出现'\u'情况，准备解析Unicode字符
					p = json_parse_hex4(p, &uInt);
					if(NULL == p)
					{
						top = head;
						return JSON_PARSE_INVALID_UNICODE_HEX;
					}
					//对于uInt在[0xD800, 0xDBFF]范围的字符，认为是高代理，进行代理对的解析
					//高代理项范围[0xD800, 0xDBFF]；低代理项范围[0xDC00, 0xDFFF]
					if(uInt <= 0xDBFF && 0xD800 <= uInt)
					{
						if(*p != '\\' || *(p + 1) != 'u')
						{
							top = head;
							return JSON_PARSE_INVALID_UNICODE_SURROGATE;
						}
						p += 2;
						p = json_parse_hex4(p, &uInt2);
						if(NULL == p)
						{
							top = head;
							return JSON_PARSE_INVALID_UNICODE_HEX;
						}
						if(uInt2 < 0xDC00 || 0xDFFF < uInt2)
						{
							top = head;
							return JSON_PARSE_INVALID_UNICODE_SURROGATE;
						}
						uInt = (((uInt - 0xD800) << 10) | (uInt2 - 0xDC00)) + 0x10000;
					}
					json_encode_utf8(uInt);
					break;
					//非法的转义字符
				default:
					top = head;
					return JSON_PARSE_INVALID_STRING_ESCAPE;
				}
				break;
			case '\0':
				//缺少结尾
				top = head;
				return JSON_PARSE_MISS_QUOTATION_MARK;
			default:
				//对于ASCII值小于0x20，认为是非法字符
				if((unsigned char)ch < 0x20)
				{
					top = head;
					return JSON_PARSE_INVALID_STRING_CHAR;
				}
				//Question:程序可以无限加入字符？
				PUTC(ch);
			}
		}
	}

	//解析string数据元素
	returnType json_context::json_parse_string(json_value* v)
	{
		returnType ret;
		char* s;
		size_t len;

		ret = json_parse_string_raw(&s, &len);
		if(JSON_PARSE_OK == ret)
		{
			v->json_set_string(s, len);
			free(s);
		}
		return ret;
	}

	
	//解析array数据元素
	//array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
	returnType json_context::json_parse_array(json_value* v)
	{
		size_t size = 0;
		size_t i;
		returnType ret;
		EXPECT('[');
		//过滤空白符
		json_filter_whitespace();
		if(']' == *json)
		{
			++json;
			v->type = JSON_ARRAY;
			v->uni.arr.size = 0;
			v->uni.arr.e = NULL;
			return JSON_PARSE_OK;
		}
		while(1)
		{
			json_value val;
			ret = json_parse_value(&val);
			if(JSON_PARSE_OK != ret)
			{
				////Question : sz等于0，会报错？
				//json_context_pop(c, size * sizeof(json_value));
				//return ret;
				break;
			}
			//申请sizeof(json_value)大小的空间来存放已经解析出来的arr中的元素
			memcpy(json_context_push(sizeof(json_value)), &val, sizeof(json_value));
			//增加一个数组元素
			++size;
			json_filter_whitespace();
			if(',' == *json)
			{
				++json;
				json_filter_whitespace();
			}
			else if(']' == *json)
			{
				++json;
				v->type = JSON_ARRAY;
				v->uni.arr.size = size;
				//size表示元素的个数， sz表示所有元素所占空间的大小
				int sz = size * sizeof(json_value);
				//对JSON元素v里面的数组的指针分配sz大小空间，用来存放size个json_value元素，此时未赋值
				v->uni.arr.e = (json_value*)malloc(sz);
				//将JSON上下文的c的栈顶top后移sz,同时将c的栈赋值给v->uni.arr.e指向的空间
				memcpy((void*)v->uni.arr.e, json_context_pop(sz), sz);
				return JSON_PARSE_OK;
			}
			else
			{
				//缺少逗号以及右方括号情况
				//比如：'\0', '{'等情况
				ret = JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
				break;
			}
		}
		//NOTE:这里size可能为0
		for(i = 0; i < size; ++i)
		{
			json_value* tmp = reinterpret_cast<json_value*>(json_context_pop(sizeof(json_value)));
			tmp->json_free();
		}
		return ret;
	}

	//释放成员
	static void json_free_member(json_member* m)
	{
		assert(NULL != m);
		free(m->key);
		m->val.json_free();
	}

	//解析object数据元素
	returnType json_context::json_parse_object(json_value* v)
	{
		size_t i, size = 0;
		size_t sz;
		json_member member;
		returnType ret;
		EXPECT('{');
		json_filter_whitespace();
		//情况1：或者有'}', 则为空对象，直接返回, size=0
		if('}' == *json)
		{
			++json;
			v->type = JSON_OBJECT;
			v->uni.obj.m = NULL;
			v->uni.obj.size = 0;
			return JSON_PARSE_OK;
		}
		member.key = NULL;
		while(1)
		{
			member.val.init();
			//情况2，首先解析关键字，所以期待'"'；如果没有出现'"',则ret = JSON_PARSE_MISS_KEY;
			if('\"' != *json)
			{
				ret = JSON_PARSE_MISS_KEY;
				break;
			}
			ret = json_parse_string_raw(&member.key, &member.keyLen);
			if(JSON_PARSE_OK != ret)
			{
				break;
			}
			json_filter_whitespace();
			if(':' != *json)
			{
				ret = JSON_PARSE_MISS_COLON;
				break;
			}
			++json;
			json_filter_whitespace();
			ret = json_parse_value(&member.val);
			if(JSON_PARSE_OK != ret)
			{
				break;
			}
			memcpy(json_context_push(sizeof(json_member)), &member, sizeof(json_member));
			//到此，一个对象的解析成功，size+1
			++size;
			//member.key初始化
			member.key = NULL;
			json_filter_whitespace();
			if(',' == *json)
			{
				++json;
				json_filter_whitespace();
			}
			else if('}' == *json)
			{
				++json;
				v->type = JSON_OBJECT;
				v->uni.obj.size = size;
				sz = size * sizeof(json_member);
				v->uni.obj.m = (json_member*)malloc(sz);
				memcpy((void *)v->uni.obj.m, json_context_pop(sz), sz);
				return JSON_PARSE_OK;
			}
			else
			{
				ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
				break;
			}
		}
		//NOTE:容易忘记,释放key
		free(member.key);
		for(i = 0; i < size; ++i)
		{
			json_free_member((json_member*)json_context_pop(sizeof(json_member)));
		}
		return ret;
	}

	//解析json的数据类型    ---依据首字符，确定解析json的数据类型方法
	returnType  json_context::json_parse_value(json_value* v)
	{
		switch(*json)
		{
		case 'n':
			//尝试解析null
			return json_parse_literal(v, "null", JSON_NULL);
		case 't':
			//尝试解析true
			return json_parse_literal(v, "true", JSON_TRUE);
		case 'f':
			//尝试解析false
			return json_parse_literal(v, "false", JSON_FALSE);
		case '\0':
			//JSON字符串为本身为空
			return JSON_PARSE_EXPECT_VALUE;
		case '\"':
			//尝试解析string类型
			return json_parse_string(v);
		case '[':
			//尝试解析array类型
			return json_parse_array(v);
		case '{':
			//尝试解析object类型
			return json_parse_object(v);
		default:
			//不合法的字符---暂时将这里改为解析number类型（虽然有点问题）
			return json_parse_number(v);
		}
	}

	//解析json数据文本
	returnType json_value::json_parse(const char* json)
	{
		json_context cntxt;
		returnType ret;
		//断言v不为空
		assert(NULL != this);
		//初始化上下文c中的json文本指针，栈指针，栈空间大小，堆顶
		cntxt.json = json;
		cntxt.stack = NULL;
		cntxt.size = cntxt.top = 0;
		//过滤上下文中前面的空白符
		cntxt.json_filter_whitespace();
		//解析cntxt信息，解析结果返回ret
		ret = cntxt.json_parse_value(this);
		if(JSON_PARSE_OK == ret)
		{
			//继续过滤空白符
			cntxt.json_filter_whitespace();
			//如果cntxt文本仍然存在待解析的文本
			if(*cntxt.json)
			{
				ret = JSON_PARSE_ROOT_NOT_SINGULAR;
			}
		}
		if(JSON_PARSE_OK != ret)
		{
			type = JSON_ERROR;
		}
		//防止内存泄漏
		assert(0 == cntxt.top);
		free(cntxt.stack);
		return ret;
	}

	void json_context::json_stringify_string(const char* s, size_t len)
	{
		static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		size_t i, size;
		char* head, *p;
		assert(NULL != s);
		size = len * 6 + 2;
		p = head = reinterpret_cast<char*>(json_context_push(size));
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
				//注意不是*p++='\b';
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
		//p-head得到真正生成字符串的长度（带有两个引号的）
		//c->top更新为栈顶位置指向下一个字符串元素的位置，栈元素就是生成的字符串
		top -= size - (p - head);
	}

	void json_context::json_stringify_array(const json_value* e, size_t size)
	{
		size_t i = 0;
		PUTC('[');
		for(; i < size; ++i)
		{
			json_stringify_value(e + i);
			if(size - 1 != i)
			{
				PUTC(',');
			}
		}
		PUTC(']');
	}

	void json_context::json_stringify_member(const json_member* m)
	{
		json_stringify_string(m->key, m->keyLen);
		PUTC(':');
		json_stringify_value(&m->val);
	}

	void json_context::json_stringify_object(const json_member* m, size_t size)
	{
		size_t i = 0;
		PUTC('{');
		for(; i < size; ++i)
		{
			json_stringify_member(m + i);
			if(size - 1 != i)
			{
				PUTC(',');
			}
		}
		PUTC('}');
	}

	void json_context::json_stringify_value(const json_value* v)
	{
		switch(v->type)
		{
		case JSON_NULL:
			PUTS("null", 4);
			break;
		case JSON_TRUE:
			PUTS("true", 4);
			break;
		case JSON_FALSE:
			PUTS("false", 5);
			break;
		case JSON_NUMBER:
			top -= 32 - sprintf(reinterpret_cast<char*>(json_context_push(32)),
								"%.17g", v->uni.num);
			break;
		case JSON_STRING:
			json_stringify_string(v->uni.str.s, v->uni.str.len);
			break;
		case JSON_ARRAY:
			json_stringify_array(v->uni.arr.e, v->uni.arr.size);
			break;
		case JSON_OBJECT:
			json_stringify_object(v->uni.obj.m, v->uni.obj.size);
			break;
		default:
			assert(0 && "invalid type");
		}
	}

	char* json_value::json_stringify(size_t* l)const
	{
		json_context cntxt;
		assert(NULL != this);
		cntxt.stack = (char*)malloc(JSON_PARSE_STRINGIFY_INIT_SIZE);
		cntxt.size = JSON_PARSE_STRINGIFY_INIT_SIZE;
		cntxt.top = 0;
		cntxt.json_stringify_value(this);
		if(l)
		{
			*l = cntxt.top;
		}
		cntxt.PUTC('\0');
		return cntxt.stack;
	}


	//返回JSON的类型
	json_type json_value::json_get_type()const
	{
		assert(NULL != this);
		return type;
	}

	//释放JSON元素所占用的空间，并将元素类型设置为JSON_ERROR
	void json_value::json_free()
	{
		assert(NULL != this);
		size_t i;
		switch(type)
		{
			//如果v的类型是string，直接释放空间
		case JSON_STRING:
			free(uni.str.s);
			break;
			//如果v的类型是array，递归释放所有数组元素的空间
		case JSON_ARRAY:
			for(i = 0; i < uni.arr.size; ++i)
			{
				(uni.arr.e + i)->json_free();
			}
			free(uni.arr.e);
			break;
		case JSON_OBJECT:
			for(i = 0; i < uni.obj.size; ++i)
			{
				json_free_member(uni.obj.m + i);
			}
			free(uni.arr.e);
		default:
			break;
		}
		type = JSON_ERROR;
	}

	//----------------获取JSON元素值和设置JSON元素值的接口---------
	json_type json_value::json_get_null()const
	{
		assert(NULL != this && (JSON_NULL == type));
		return type;
	}

	void json_value::json_set_null()
	{
		json_free();
		type = JSON_NULL;
	}

	json_type json_value::json_get_boolean()const
	{
		assert(NULL != this && (JSON_TRUE == type || JSON_FALSE == type));
		return type;
	}

	void json_value::json_set_boolean(json_type b)
	{
		json_free();
		type = (b == JSON_TRUE) ? JSON_TRUE : JSON_FALSE;
	}

	double json_value::json_get_number()const
	{
		assert(NULL != this&&JSON_NUMBER == type);
		return uni.num;
	}

	void json_value::json_set_number(double num)
	{
		json_free();
		uni.num = num;
		type = JSON_NUMBER;
	}

	const char* json_value::json_get_string()const
	{
		assert(NULL != this&& JSON_STRING == type);
		return uni.str.s;
	}

	size_t json_value::json_get_string_length()const
	{
		assert(NULL != this && JSON_STRING == type);
		return uni.str.len;
	}

	void json_value::json_set_string(const char* s, size_t len)
	{
		assert(s != NULL || 0 == len);
		json_free();
		uni.str.s = (char*)malloc(len + 1);
		memcpy(uni.str.s, s, len);
		uni.str.s[len] = '\0';
		uni.str.len = len;
		type = JSON_STRING;
	}

	json_value* json_value::json_get_array_element(size_t index)const
	{
		assert(NULL != this&&JSON_ARRAY == type&&index < uni.arr.size);
		return &uni.arr.e[index];
	}

	size_t json_value::json_get_array_size()const
	{
		assert(NULL != this &&JSON_ARRAY == type);
		return uni.arr.size;
	}

	const char* json_value::json_get_object_key(size_t index)const
	{
		assert(NULL != this&&JSON_OBJECT == type);
		assert(index < uni.obj.size);
		return uni.obj.m[index].key;
	}

	size_t json_value::json_get_object_key_length(size_t index)const
	{
		assert(NULL != this&&JSON_OBJECT == type);
		assert(index < uni.obj.size);
		return uni.obj.m[index].keyLen;
	}

	json_value* json_value::json_get_object_value(size_t index)const
	{
		assert(NULL != this&&JSON_OBJECT == type);
		assert(index < uni.obj.size);
		return &uni.obj.m[index].val;
	}

	size_t json_value::json_get_object_size()const
	{
		assert(NULL != this&&JSON_OBJECT == type);
		return uni.obj.size;
	}
}