//#include ����  �����ظ�����
#ifndef LEPTJSON_H_
#define LEPTJSON_H_
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
}lept_value;

/*
 * LEPT_PARSE_OK��ʾ�����ɹ�
 * LEPT_PARSE_EXPECT_VALUE��ʾ�ڴ�ֵ���֣���Ϊ�Կհ��ַ����ķ���ֵ
 * LEPT_PARSE_INVALID_VALUE��ʾ���ֲ������﷨���ַ�
 * LEPT_PARSE_ROOT_NOT_SINGULAR��ʾ�ɹ�����֮�������ַ�Ҫ����
 */
enum returnType
{
	LEPT_PARSE_OK, LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE, LEPT_PARSE_ROOT_NOT_SINGULAR
};
//����JSON�ַ���������������Ľӿ�
int lept_parse(lept_value* v, const char* json);
//��ȡJSONԪ�ص���������
lept_type lept_get_type(const lept_value* v);

#endif