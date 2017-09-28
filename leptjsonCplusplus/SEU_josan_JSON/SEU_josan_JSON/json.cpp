/*
 * ---------------------------------------------------
 *  Copyright (c) 2017 josan All rights reserved.
 * ---------------------------------------------------
 *
 *               �����ߣ� Josan
 *             ����ʱ�䣺 2017/9/28 10:36:52
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
 //�ṩ14��API,����������staticʵ�ֿ�ķ�װ
#include "json.h"
 //-----------------�궨�岿��----------------
#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef JSON_PARSE_STRINGIFY_INIT_SIZE
#define JSON_PARSE_STRINGIFY_INIT_SIZE 256
#endif

namespace SEU_josan_JSON
{

	//�ж�ch�Ƿ���1-9֮��
	inline bool isDigit1to9(char ch)
	{
		return (ch) <= '9' && '1' <= (ch);
	}
	//�ж�ch�Ƿ���16������
	inline bool isHex(char ch)
	{
		return ((ch) <= '9' && '0' <= (ch)) ||
			((toupper(ch) <= 'F' && 'A' <= toupper(ch)));
	}
	//��������c��ջ������sz�����ջ������ÿ������1.5���ռ䣬ֱ������Ҫ��
	//����ֵ��֮ǰջ��ջ������һ��Ԫ��
	//c->stackָ��û�䣬
	//c->sz��ջ����ʱ�򣬻����ӣ�������ʱ�򣬲��䡣
	//c->top�����ı䡣
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
				//�ȼ���c->size*=1.5;
				size += size >> 1;
			}
			stack = (char*)realloc(stack, size);
		}
		ret = stack + top;
		top += sz;
		return ret;
	}
	//���˿հ׷� ws = *(%x20 / %x09 / %x0A / %x0D)
	void json_context::json_filter_whitespace()
	{
		const char* p = json;
		//������ò�Ҫ��<ctype.h>��isspace()
		while(' ' == *p || '\t' == *p || '\n' == *p || '\r' == *p)
		{
			++p;
		}
		json = p;
	}

	//����null, true, false���� cΪjson�ı���vΪ����֮��jsonԪ�أ�
	//literalΪԤ�ڽ������ַ����� typeΪ�����ɹ�֮���Ԫ������
	returnType json_context::json_parse_literal(json_value* v,
								  const char* literal, json_type type)
	{
		size_t i;
		//�ڴ�����literal[0]��ͬʱ++c->json
		EXPECT(literal[0]);
		//�����У��������Ϸ����ַ�������JSON_PARSE_INVALID_VALUE
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

	//����number����
	returnType json_context::json_parse_number(json_value* v)
	{

		const char* p = json;
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
			if(!isDigit1to9(*p))
			{
				return JSON_PARSE_INVALID_VALUE;
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
				return JSON_PARSE_INVALID_VALUE;
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
				return JSON_PARSE_INVALID_VALUE;
			}
			//������������
			for(++p; isdigit(*p); ++p);
		}
		v->type = JSON_NUMBER;
		errno = 0;
		//-----------------------ע��ֱ����strtod()�Ὣ��಻�Ϸ�����ֵ�Ͷ���ȷת��
		////end��������strtod()�Ƿ�����ɹ�,���Գɹ�������ȥ
		/*char* end;
		v->num = strtod(c->json, &end);
		if(c->json == end)
		{
		return JSON_PARSE_INVALID_VALUE;
		}
		c->json = end;*/
		//-------------------------
		v->uni.num = strtod(json, NULL);
		//��������������������errno��ΪERANGE�����ǳ�����Χ��ǣ�
		//�����HUGE_VAL == v->num || -HUGE_VAL == v->num����ȷ�������������inf��
		//���Ǹ�������� -inf
		if(errno == ERANGE && (HUGE_VAL == v->uni.num || -HUGE_VAL == v->uni.num))
		{
			return JSON_PARSE_NUMBER_TOO_BIG;
		}
		json = p;
		return JSON_PARSE_OK;
	}

	//����/uXXXX�ġ�XXXX�����֣�����ֵ�浽*u����
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

	//��Unicode�ַ�����UTF8�ı��뷽�����б���
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
	
	//�ع�����string����Ԫ��
	//����ԭ����string�ַ�������JSONstring��һ���֣����漰json_value* v�ĸ�ֵ

	returnType json_context::json_parse_string_raw(char** s, size_t* l)
	{
		//ע��˴�������head����Ϊhead���ܲ�Ϊ0�������������ʱ��������c��ջ�����Ѿ�����Ԫ�أ����head���ܲ�Ϊ0��
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
				//���ս�β�ַ�
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
				//����ת���ַ�
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
					//����'\u'�����׼������Unicode�ַ�
					p = json_parse_hex4(p, &uInt);
					if(NULL == p)
					{
						top = head;
						return JSON_PARSE_INVALID_UNICODE_HEX;
					}
					//����uInt��[0xD800, 0xDBFF]��Χ���ַ�����Ϊ�Ǹߴ������д���ԵĽ���
					//�ߴ����Χ[0xD800, 0xDBFF]���ʹ����Χ[0xDC00, 0xDFFF]
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
					//�Ƿ���ת���ַ�
				default:
					top = head;
					return JSON_PARSE_INVALID_STRING_ESCAPE;
				}
				break;
			case '\0':
				//ȱ�ٽ�β
				top = head;
				return JSON_PARSE_MISS_QUOTATION_MARK;
			default:
				//����ASCIIֵС��0x20����Ϊ�ǷǷ��ַ�
				if((unsigned char)ch < 0x20)
				{
					top = head;
					return JSON_PARSE_INVALID_STRING_CHAR;
				}
				//Question:����������޼����ַ���
				PUTC(ch);
			}
		}
	}

	//����string����Ԫ��
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

	
	//����array����Ԫ��
	//array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
	returnType json_context::json_parse_array(json_value* v)
	{
		size_t size = 0;
		size_t i;
		returnType ret;
		EXPECT('[');
		//���˿հ׷�
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
				////Question : sz����0���ᱨ��
				//json_context_pop(c, size * sizeof(json_value));
				//return ret;
				break;
			}
			//����sizeof(json_value)��С�Ŀռ�������Ѿ�����������arr�е�Ԫ��
			memcpy(json_context_push(sizeof(json_value)), &val, sizeof(json_value));
			//����һ������Ԫ��
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
				//size��ʾԪ�صĸ����� sz��ʾ����Ԫ����ռ�ռ�Ĵ�С
				int sz = size * sizeof(json_value);
				//��JSONԪ��v����������ָ�����sz��С�ռ䣬�������size��json_valueԪ�أ���ʱδ��ֵ
				v->uni.arr.e = (json_value*)malloc(sz);
				//��JSON�����ĵ�c��ջ��top����sz,ͬʱ��c��ջ��ֵ��v->uni.arr.eָ��Ŀռ�
				memcpy((void*)v->uni.arr.e, json_context_pop(sz), sz);
				return JSON_PARSE_OK;
			}
			else
			{
				//ȱ�ٶ����Լ��ҷ��������
				//���磺'\0', '{'�����
				ret = JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
				break;
			}
		}
		//NOTE:����size����Ϊ0
		for(i = 0; i < size; ++i)
		{
			json_value* tmp = reinterpret_cast<json_value*>(json_context_pop(sizeof(json_value)));
			tmp->json_free();
		}
		return ret;
	}

	//�ͷų�Ա
	static void json_free_member(json_member* m)
	{
		assert(NULL != m);
		free(m->key);
		m->val.json_free();
	}

	//����object����Ԫ��
	returnType json_context::json_parse_object(json_value* v)
	{
		size_t i, size = 0;
		size_t sz;
		json_member member;
		returnType ret;
		EXPECT('{');
		json_filter_whitespace();
		//���1��������'}', ��Ϊ�ն���ֱ�ӷ���, size=0
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
			//���2�����Ƚ����ؼ��֣������ڴ�'"'�����û�г���'"',��ret = JSON_PARSE_MISS_KEY;
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
			//���ˣ�һ������Ľ����ɹ���size+1
			++size;
			//member.key��ʼ��
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
		//NOTE:��������,�ͷ�key
		free(member.key);
		for(i = 0; i < size; ++i)
		{
			json_free_member((json_member*)json_context_pop(sizeof(json_member)));
		}
		return ret;
	}

	//����json����������    ---�������ַ���ȷ������json���������ͷ���
	returnType  json_context::json_parse_value(json_value* v)
	{
		switch(*json)
		{
		case 'n':
			//���Խ���null
			return json_parse_literal(v, "null", JSON_NULL);
		case 't':
			//���Խ���true
			return json_parse_literal(v, "true", JSON_TRUE);
		case 'f':
			//���Խ���false
			return json_parse_literal(v, "false", JSON_FALSE);
		case '\0':
			//JSON�ַ���Ϊ����Ϊ��
			return JSON_PARSE_EXPECT_VALUE;
		case '\"':
			//���Խ���string����
			return json_parse_string(v);
		case '[':
			//���Խ���array����
			return json_parse_array(v);
		case '{':
			//���Խ���object����
			return json_parse_object(v);
		default:
			//���Ϸ����ַ�---��ʱ�������Ϊ����number���ͣ���Ȼ�е����⣩
			return json_parse_number(v);
		}
	}

	//����json�����ı�
	returnType json_value::json_parse(const char* json)
	{
		json_context cntxt;
		returnType ret;
		//����v��Ϊ��
		assert(NULL != this);
		//��ʼ��������c�е�json�ı�ָ�룬ջָ�룬ջ�ռ��С���Ѷ�
		cntxt.json = json;
		cntxt.stack = NULL;
		cntxt.size = cntxt.top = 0;
		//������������ǰ��Ŀհ׷�
		cntxt.json_filter_whitespace();
		//����cntxt��Ϣ�������������ret
		ret = cntxt.json_parse_value(this);
		if(JSON_PARSE_OK == ret)
		{
			//�������˿հ׷�
			cntxt.json_filter_whitespace();
			//���cntxt�ı���Ȼ���ڴ��������ı�
			if(*cntxt.json)
			{
				ret = JSON_PARSE_ROOT_NOT_SINGULAR;
			}
		}
		if(JSON_PARSE_OK != ret)
		{
			type = JSON_ERROR;
		}
		//��ֹ�ڴ�й©
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


	//����JSON������
	json_type json_value::json_get_type()const
	{
		assert(NULL != this);
		return type;
	}

	//�ͷ�JSONԪ����ռ�õĿռ䣬����Ԫ����������ΪJSON_ERROR
	void json_value::json_free()
	{
		assert(NULL != this);
		size_t i;
		switch(type)
		{
			//���v��������string��ֱ���ͷſռ�
		case JSON_STRING:
			free(uni.str.s);
			break;
			//���v��������array���ݹ��ͷ���������Ԫ�صĿռ�
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

	//----------------��ȡJSONԪ��ֵ������JSONԪ��ֵ�Ľӿ�---------
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