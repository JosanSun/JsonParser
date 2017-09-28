/*
* ---------------------------------------------------
*  Copyright (c) 2017 josan All rights reserved.
* ---------------------------------------------------
*
*               创建者： Josan
*             创建时间： 2017/9/28 10:36:52
*/
//#include 防范  避免重复声明
#ifndef JSON_H_
#define JSON_H_
#include <cassert>
#include <cstddef>     //size_t
#include <cstring>    //memcpy()

//json的六种数据类型：null, boolean(false, true), number, 
// string, array, object  
//另外添加一个数据类型json_ERROR。当JSON解析错误时，其对应的数据类型为json_ERROR

namespace SEU_josan_JSON
{

	enum json_type
	{
		JSON_NULL, JSON_FALSE, JSON_TRUE, JSON_NUMBER,
		JSON_STRING, JSON_ARRAY, JSON_OBJECT, JSON_ERROR
	};
	/*
	* returnType表示解析的返回值枚举
	* json_PARSE_OK表示解析成功
	* json_PARSE_EXPECT_VALUE表示期待值出现，作为对空白字符串的返回值
	* json_PARSE_INVALID_VALUE表示出现不符合语法的字符
	* json_PARSE_ROOT_NOT_SINGULAR表示成功解析之后，仍有字符要解析
	* json_PARSE_NUMBER_TOO_BIG表示当解析数值时，解析出来的数值超过最大所能表示的范围
	* json_PARSE_MISS_QUOTATION_MARK表示缺少引号标志(只有前面的引号，缺少后面一个引号)
	* json_PARSE_INVALID_STRING_ESCAPE表示出现不合法的转义字符
	* json_PARSE_INVALID_STRING_CHAR表示出现ASCII码值小于0x20的字符
	* json_PARSE_INVALID_UNICODE_HEX表示解析字符串时，Unicode中出现非法的16进制字符
	* json_PARSE_INVALID_UNICODE_SURROGATE非法的代理对
	* json_PARSE_MISS_COMMA_OR_SQUARE_BRACKET表示解析数组时，数组元素缺少逗号或者']'
	* json_PARSE_MISS_KEY表示解析对象时，对象元素缺少相应的关键字
	* json_PARSE_MISS_COLON表示解析对象时，对象元素缺少冒号':'
	* json_PARSE_MISS_COMMA_OR_CURLY_BRACKET表示解析对象时，对象元素缺少缺少逗号','或者大括号'}'
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
		//解析JSON字符的主函数，对外的接口
		returnType json_parse(const char* json);
		//利用json元素生成字符串序列  JSON的生成器部分
		char* json_stringify(size_t* l)const;
		//获取JSON元素的数据类型
		json_type json_get_type()const;
		//如果v是STRING类型就是释放所占用的空间，不是的话，统一将v的类型置为初始化类型
		void json_free();

		//----------------获取JSON元素值和设置JSON元素值的接口---------
		//null类型
		json_type json_get_null()const;
		void json_set_null();
		//boolean元素
		json_type json_get_boolean()const;
		void json_set_boolean(json_type b);
		//number元素
		double json_get_number()const;
		void json_set_number(double num);
		//string 元素
		const char* json_get_string()const;
		size_t json_get_string_length()const;
		void json_set_string(const char* s, size_t len);
		//array 元素
		json_value* json_get_array_element(size_t index)const;
		size_t json_get_array_size()const;
		//object元素
		const char* json_get_object_key(size_t index)const;
		size_t json_get_object_key_length(size_t index)const;
		json_value* json_get_object_value(size_t index)const;
		size_t json_get_object_size()const;
	private:
		json_type type;
		//由于json数值不可能同时为string和number类型，因此用共用体来表示
		union
		{
			//num用来保存json为数值型的值
			double num;
			//str用来保存string的头指针和长度
			struct
			{
				char* s;
				size_t len;
			}str;
			//arr用来保存array的数组元素(注意是json_value)首地址和数组大小
			struct
			{
				json_value* e;
				size_t size;
			}arr;
			//obj用来保存object的对象元素，包括对象成员json_member，以及对象数量size
			struct
			{
				json_member* m;
				size_t size;
			}obj;
		}uni;
	};
	//JSON对象成员的类型， <key, val>一个键值对
	struct json_member
	{
		char* key;
		size_t keyLen;
		json_value val;
	};

	//要解析的上下文文本类型
	//为了存储解析到的字符串，设置一个stack,
	//其中top指向栈顶元素的下一个位置，size表示栈的空间大小， size和top的单位都是字节
	//栈空为top==0, 栈满为top == size
	class json_context
	{
		friend class json_value;
	public:
		void* json_context_push(size_t sz);
		void json_filter_whitespace();
		returnType json_parse_literal(json_value* v,
									  const char* literal, json_type type);
		returnType json_parse_number(json_value* v);
		//解析string数据元素
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
		//在JSON上下文c的栈顶，添加字符ch
		void PUTC(char ch)
		{
			char* tem = reinterpret_cast<char*>(json_context_push(sizeof(char)));
			*tem = ch;
		}
		void PUTS(char* s, int len)
		{
			memcpy(json_context_push(len), s, len);
		}
		//断言字符串c的首字符是ch，并将字符向后移动一位
		void EXPECT(char ch)
		{
			assert(*json == ch);
			++json;
		}

		//函数功能：删除sz个栈元素
		//栈的元素个数大于待删除的个数
		//返回删除之后的栈顶元素的下一个位置
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

