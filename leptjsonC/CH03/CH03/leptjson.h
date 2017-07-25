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

//JSON����Ԫ�أ������ݽṹ��JSON��һ�����νṹ
typedef struct
{
	lept_type type;
	//����json��ֵ������ͬʱΪstring��number���ͣ�����ù���������ʾ
	union
	{
		//str��������string��ͷָ��ͳ���
		struct
		{
			char* s;
			size_t len;
		}str;
		//num��������jsonΪ��ֵ�͵�ֵ
		double num;
	}uni;
}lept_value;

/*
 * LEPT_PARSE_OK��ʾ�����ɹ�
 * LEPT_PARSE_EXPECT_VALUE��ʾ�ڴ�ֵ���֣���Ϊ�Կհ��ַ����ķ���ֵ
 * LEPT_PARSE_INVALID_VALUE��ʾ���ֲ������﷨���ַ�
 * LEPT_PARSE_ROOT_NOT_SINGULAR��ʾ�ɹ�����֮�������ַ�Ҫ����
 * LEPT_PARSE_NUMBER_TOO_BIG��ʾ��������ֵʱ��������������ֵ����������ܱ�ʾ�ķ�Χ
 * LEPT_PARSE_MISS_QUOTATION_MARK��ʾȱ�����ű�־(ֻ��ǰ������ţ�ȱ�ٺ���һ������)
 * LEPT_PARSE_INVALID_STRING_ESCAPE��ʾ���ֲ��Ϸ���ת���ַ�
 * LEPT_PARSE_INVALID_STRING_CHAR��ʾ����ASCII��ֵС��0x20���ַ�
 */
enum returnType
{
	LEPT_PARSE_OK, LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE, LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG, LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE, LEPT_PARSE_INVALID_STRING_CHAR
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

#endif
