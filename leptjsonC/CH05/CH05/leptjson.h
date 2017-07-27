//#include ����  �����ظ�����
#ifndef LEPTJSON_H_
#define LEPTJSON_H_

#include <stddef.h>     //size_t

//josan�������������ͣ�null, boolean(false, true), number, 
// string, array, object  
//�������һ����������LEPT_ERROR����JSON��������ʱ�����Ӧ����������ΪLEPT_ERROR
typedef enum
{
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER,
	LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_ERROR
}lept_type;

//���������У���ǰ����ָ��lept_value��ָ�룬���Ա�����ǰ����
//����ֱ������struct lept_value; ����ʹ��typedef������
typedef struct lept_value lept_value;

//JSON����Ԫ�أ������ݽṹ��JSON��һ�����νṹ
struct lept_value
{
	lept_type type;
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
		//arr��������array������Ԫ��(ע����lept_value)�׵�ַ�������С
		struct
		{
			lept_value* e;
			size_t size;
		}arr;
	}uni;
};

/*
 * returnType��ʾ�����ķ���ֵö��
 * ------------ CH01 ----------------
 * LEPT_PARSE_OK��ʾ�����ɹ�
 * LEPT_PARSE_EXPECT_VALUE��ʾ�ڴ�ֵ���֣���Ϊ�Կհ��ַ����ķ���ֵ
 * LEPT_PARSE_INVALID_VALUE��ʾ���ֲ������﷨���ַ�
 * LEPT_PARSE_ROOT_NOT_SINGULAR��ʾ�ɹ�����֮�������ַ�Ҫ����
 * ------------ CH02 ----------------
 * LEPT_PARSE_NUMBER_TOO_BIG��ʾ��������ֵʱ��������������ֵ����������ܱ�ʾ�ķ�Χ
 * ------------ CH03 ----------------
 * LEPT_PARSE_MISS_QUOTATION_MARK��ʾȱ�����ű�־(ֻ��ǰ������ţ�ȱ�ٺ���һ������)
 * LEPT_PARSE_INVALID_STRING_ESCAPE��ʾ���ֲ��Ϸ���ת���ַ�
 * LEPT_PARSE_INVALID_STRING_CHAR��ʾ����ASCII��ֵС��0x20���ַ�
 * ------------ CH04 ----------------
 * LEPT_PARSE_INVALID_UNICODE_HEX��ʾ�����ַ���ʱ��Unicode�г��ַǷ���16�����ַ�
 * LEPT_PARSE_INVALID_UNICODE_SURROGATE�Ƿ��Ĵ����
 * ------------ CH05 ----------------
 * LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET��ʾ��������ʱ������Ԫ��ȱ�ٶ��Ż���']'
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

//��ʼ��jsonԪ�أ�����Ԫ�ص����Ͷ���ΪLEPT_ERROR
#define lept_init(v) do{ (v)->type = LEPT_ERROR; }while(0)

//����JSON�ַ���������������Ľӿ�
int lept_parse(lept_value* v, const char* json);
//��ȡJSONԪ�ص���������
lept_type lept_get_type(const lept_value* v);
//���v��STRING���;����ͷ���ռ�õĿռ䣬���ǵĻ���ͳһ��v��������Ϊ��ʼ������
void lept_free(lept_value* v);

//----------------��ȡJSONԪ��ֵ������JSONԪ��ֵ�Ľӿ�---------
//null����
int lept_get_null(const lept_value* v);
void lept_set_null(lept_value* v);
//booleanԪ��
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);
//numberԪ��
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double num);
//string Ԫ��
const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
//array Ԫ��
lept_value* lept_get_array_element(const lept_value* v, size_t index);
size_t lept_get_array_length(const lept_value* v);
#endif
