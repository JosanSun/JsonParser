//#include 防范  避免重复声明
#ifndef LEPTJSON_H_
#define LEPTJSON_H_
//josan的六种数据类型：null, boolean(false, true), number, 
// string, array, object  
//另外添加一个数据类型LEPT_ERROR。当JSON解析错误时，其对应的数据类型为LEPT_ERROR
typedef enum
{
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER,
	LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_ERROR
}lept_type;

//JSON数据元素，或数据结构，JSON是一个树形结构
typedef struct
{
	lept_type type;
}lept_value;

/*
 * LEPT_PARSE_OK表示解析成功
 * LEPT_PARSE_EXPECT_VALUE表示期待值出现，作为对空白字符串的返回值
 * LEPT_PARSE_INVALID_VALUE表示出现不符合语法的字符
 * LEPT_PARSE_ROOT_NOT_SINGULAR表示成功解析之后，仍有字符要解析
 */
enum returnType
{
	LEPT_PARSE_OK, LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE, LEPT_PARSE_ROOT_NOT_SINGULAR
};
//解析JSON字符的主函数，对外的接口
int lept_parse(lept_value* v, const char* json);
//获取JSON元素的数据类型
lept_type lept_get_type(const lept_value* v);

#endif