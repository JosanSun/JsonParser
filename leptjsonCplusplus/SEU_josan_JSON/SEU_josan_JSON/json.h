/*
* ---------------------------------------------------
*  Copyright (c) 2017 josan All rights reserved.
* ---------------------------------------------------
*
*               �����ߣ� Josan
*             ����ʱ�䣺 2017/9/28 10:36:52
*/
//#include ����  �����ظ�����
#ifndef JSON_H_
#define JSON_H_
#include <cassert>
#include <cstddef>     //size_t
#include <cstring>    //memcpy()

//json�������������ͣ�null, boolean(false, true), number, 
// string, array, object  
//�������һ����������json_ERROR����JSON��������ʱ�����Ӧ����������Ϊjson_ERROR

namespace SEU_josan_JSON
{

	enum json_type
	{
		JSON_NULL, JSON_FALSE, JSON_TRUE, JSON_NUMBER,
		JSON_STRING, JSON_ARRAY, JSON_OBJECT, JSON_ERROR
	};
	/*
	* returnType��ʾ�����ķ���ֵö��
	* json_PARSE_OK��ʾ�����ɹ�
	* json_PARSE_EXPECT_VALUE��ʾ�ڴ�ֵ���֣���Ϊ�Կհ��ַ����ķ���ֵ
	* json_PARSE_INVALID_VALUE��ʾ���ֲ������﷨���ַ�
	* json_PARSE_ROOT_NOT_SINGULAR��ʾ�ɹ�����֮�������ַ�Ҫ����
	* json_PARSE_NUMBER_TOO_BIG��ʾ��������ֵʱ��������������ֵ����������ܱ�ʾ�ķ�Χ
	* json_PARSE_MISS_QUOTATION_MARK��ʾȱ�����ű�־(ֻ��ǰ������ţ�ȱ�ٺ���һ������)
	* json_PARSE_INVALID_STRING_ESCAPE��ʾ���ֲ��Ϸ���ת���ַ�
	* json_PARSE_INVALID_STRING_CHAR��ʾ����ASCII��ֵС��0x20���ַ�
	* json_PARSE_INVALID_UNICODE_HEX��ʾ�����ַ���ʱ��Unicode�г��ַǷ���16�����ַ�
	* json_PARSE_INVALID_UNICODE_SURROGATE�Ƿ��Ĵ����
	* json_PARSE_MISS_COMMA_OR_SQUARE_BRACKET��ʾ��������ʱ������Ԫ��ȱ�ٶ��Ż���']'
	* json_PARSE_MISS_KEY��ʾ��������ʱ������Ԫ��ȱ����Ӧ�Ĺؼ���
	* json_PARSE_MISS_COLON��ʾ��������ʱ������Ԫ��ȱ��ð��':'
	* json_PARSE_MISS_COMMA_OR_CURLY_BRACKET��ʾ��������ʱ������Ԫ��ȱ��ȱ�ٶ���','���ߴ�����'}'
	*/
	enum returnType
	{
		JSON_PARSE_OK, JSON_PARSE_EXPECT_VALUE,
		JSON_PARSE_INVALID_VALUE, JSON_PARSE_ROOT_NOT_SINGULAR,
		JSON_PARSE_NUMBER_TOO_BIG, JSON_PARSE_MISS_QUOTATION_MARK,
		JSON_PARSE_INVALID_STRING_ESCAPE, JSON_PARSE_INVALID_STRING_CHAR,
		JSON_PARSE_INVALID_UNICODE_HEX, JSON_PARSE_INVALID_UNICODE_SURROGATE,
		JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, JSON_PARSE_MISS_KEY,
		JSON_PARSE_MISS_COLON, JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET
	};
	struct json_member;
	class json_context;
	class json_value
	{
	public:
		friend class json_context;
		json_value()
		{
			type = JSON_ERROR;
		}
		void init()
		{
			type = JSON_ERROR;
		}
		//����JSON�ַ���������������Ľӿ�
		returnType json_parse(const char* json);
		//����jsonԪ�������ַ�������  JSON������������
		char* json_stringify(size_t* l)const;
		//��ȡJSONԪ�ص���������
		json_type json_get_type()const;
		//���v��STRING���;����ͷ���ռ�õĿռ䣬���ǵĻ���ͳһ��v��������Ϊ��ʼ������
		void json_free();

		//----------------��ȡJSONԪ��ֵ������JSONԪ��ֵ�Ľӿ�---------
		//null����
		json_type json_get_null()const;
		void json_set_null();
		//booleanԪ��
		json_type json_get_boolean()const;
		void json_set_boolean(json_type b);
		//numberԪ��
		double json_get_number()const;
		void json_set_number(double num);
		//string Ԫ��
		const char* json_get_string()const;
		size_t json_get_string_length()const;
		void json_set_string(const char* s, size_t len);
		//array Ԫ��
		json_value* json_get_array_element(size_t index)const;
		size_t json_get_array_size()const;
		//objectԪ��
		const char* json_get_object_key(size_t index)const;
		size_t json_get_object_key_length(size_t index)const;
		json_value* json_get_object_value(size_t index)const;
		size_t json_get_object_size()const;
	private:
		json_type type;
		//����json��ֵ������ͬʱΪstring��number���ͣ�����ù���������ʾ
		union
		{
			//num��������jsonΪ��ֵ�͵�ֵ
			double num;
			//str��������string��ͷָ��ͳ���
			struct
			{
				char* s;
				size_t len;
			}str;
			//arr��������array������Ԫ��(ע����json_value)�׵�ַ�������С
			struct
			{
				json_value* e;
				size_t size;
			}arr;
			//obj��������object�Ķ���Ԫ�أ����������Աjson_member���Լ���������size
			struct
			{
				json_member* m;
				size_t size;
			}obj;
		}uni;
	};
	//JSON�����Ա�����ͣ� <key, val>һ����ֵ��
	struct json_member
	{
		char* key;
		size_t keyLen;
		json_value val;
	};

	//Ҫ�������������ı�����
	//Ϊ�˴洢���������ַ���������һ��stack,
	//����topָ��ջ��Ԫ�ص���һ��λ�ã�size��ʾջ�Ŀռ��С�� size��top�ĵ�λ�����ֽ�
	//ջ��Ϊtop==0, ջ��Ϊtop == size
	class json_context
	{
		friend class json_value;
	public:
		void* json_context_push(size_t sz);
		void json_filter_whitespace();
		returnType json_parse_literal(json_value* v,
									  const char* literal, json_type type);
		returnType json_parse_number(json_value* v);
		//����string����Ԫ��
		returnType json_parse_string(json_value* v);
		returnType json_parse_array(json_value* v);
		returnType json_parse_object(json_value* v);
		returnType json_parse_value(json_value* v);
		void json_stringify_string(const char* s, size_t len);
		void json_stringify_array(const json_value* e, size_t size);
		void json_stringify_member(const json_member* m);
		void json_stringify_object(const json_member* m, size_t size);
		void json_stringify_value(const json_value* v);
	private:
		const char* json;
		char* stack;
		size_t size, top;
		//��JSON������c��ջ��������ַ�ch
		void PUTC(char ch)
		{
			char* tem = reinterpret_cast<char*>(json_context_push(sizeof(char)));
			*tem = ch;
		}
		void PUTS(char* s, int len)
		{
			memcpy(json_context_push(len), s, len);
		}
		//�����ַ���c�����ַ���ch�������ַ�����ƶ�һλ
		void EXPECT(char ch)
		{
			assert(*json == ch);
			++json;
		}

		//�������ܣ�ɾ��sz��ջԪ��
		//ջ��Ԫ�ظ������ڴ�ɾ���ĸ���
		//����ɾ��֮���ջ��Ԫ�ص���һ��λ��
		void* json_context_pop(size_t sz)
		{
			assert(sz <= top);
			top -= sz;
			return stack + top;
		}
		static const char* json_parse_hex4(const char* p, unsigned int* u);
		void json_encode_utf8(unsigned int uInt);
		returnType json_parse_string_raw(char** s, size_t* l);
	};
}
#endif

