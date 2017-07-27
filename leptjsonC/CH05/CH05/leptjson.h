//#include 防范  避免重复声明
#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <stddef.h>     //size_t

//josan的六种数据类型：null, boolean(false, true), number, 
// string, array, object  
//另外添加一个数据类型LEPT_ERROR。当JSON解析错误时，其对应的数据类型为LEPT_ERROR
typedef enum
{
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER,
	LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_ERROR
}lept_type;

//由于数组中，提前出现指向lept_value的指针，所以必须提前声明
//不能直接声明struct lept_value; 必须使用typedef的声明
typedef struct lept_value lept_value;

//JSON数据元素，或数据结构，JSON是一个树形结构
struct lept_value
{
	lept_type type;
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
		//arr用来保存array的数组元素(注意是lept_value)首地址和数组大小
		struct
		{
			lept_value* e;
			size_t size;
		}arr;
	}uni;
};

/*
 * returnType表示解析的返回值枚举
 * ------------ CH01 ----------------
 * LEPT_PARSE_OK表示解析成功
 * LEPT_PARSE_EXPECT_VALUE表示期待值出现，作为对空白字符串的返回值
 * LEPT_PARSE_INVALID_VALUE表示出现不符合语法的字符
 * LEPT_PARSE_ROOT_NOT_SINGULAR表示成功解析之后，仍有字符要解析
 * ------------ CH02 ----------------
 * LEPT_PARSE_NUMBER_TOO_BIG表示当解析数值时，解析出来的数值超过最大所能表示的范围
 * ------------ CH03 ----------------
 * LEPT_PARSE_MISS_QUOTATION_MARK表示缺少引号标志(只有前面的引号，缺少后面一个引号)
 * LEPT_PARSE_INVALID_STRING_ESCAPE表示出现不合法的转义字符
 * LEPT_PARSE_INVALID_STRING_CHAR表示出现ASCII码值小于0x20的字符
 * ------------ CH04 ----------------
 * LEPT_PARSE_INVALID_UNICODE_HEX表示解析字符串时，Unicode中出现非法的16进制字符
 * LEPT_PARSE_INVALID_UNICODE_SURROGATE非法的代理对
 * ------------ CH05 ----------------
 * LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET表示解析数组时，数组元素缺少逗号或者']'
 */
enum returnType
{
	LEPT_PARSE_OK, LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE, LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG, LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_INVALID_UNICODE_HEX, LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

//初始化json元素，将其元素的类型定义为LEPT_ERROR
#define lept_init(v) do{ (v)->type = LEPT_ERROR; }while(0)

//解析JSON字符的主函数，对外的接口
int lept_parse(lept_value* v, const char* json);
//获取JSON元素的数据类型
lept_type lept_get_type(const lept_value* v);
//如果v是STRING类型就是释放所占用的空间，不是的话，统一将v的类型置为初始化类型
void lept_free(lept_value* v);

//----------------获取JSON元素值和设置JSON元素值的接口---------
//null类型
int lept_get_null(const lept_value* v);
void lept_set_null(lept_value* v);
//boolean元素
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);
//number元素
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double num);
//string 元素
const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
//array 元素
lept_value* lept_get_array_element(const lept_value* v, size_t index);
size_t lept_get_array_length(const lept_value* v);
#endif
