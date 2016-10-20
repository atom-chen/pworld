#pragma once

namespace XML
{
    class CXmlAttrib;

    bool                            g_is_ascii(unsigned char ch);
    bool                            g_is_char(unsigned char ch);
    bool                            g_is_number(unsigned char ch);
    bool                            g_is_connect(unsigned char ch);
    bool                            g_is_ascii_valid(unsigned char ch);

    //xml����ʱ�õ���ȫ�ֺ���;
    size_t                          g_align_4(size_t t);


    // �ӵ�ǰλ�ÿ�ʼ�����ؽڵ�����;
    char*                           g_get_node_name( char*& pData, size_t& dwLen);

    // �Ƿ��ǽڵ�Ľ������;
    bool                            g_is_node_name_end(char* pch);
    bool                            g_is_node_name_end_1(char* pch);
    bool                            g_is_node_name_end_2(char* pch);

   
    // head�ڵ�����ı��( ?> );
    bool                            g_is_head_node_end(char*pch);

    // ��ȡ������;
    char*                           g_get_attrib_name(char*&pData, size_t& dwLen);

    // ��ȡ�ڵ�������ַ���;
    char*                           g_get_attrib_value(char*& pData, size_t& dwLen);

    // ������pAttrib���뵽lstAttrib,���ظ����;
    bool                            g_insert_attrib_list(std::list<CXmlAttrib*>& lstAttrib, CXmlAttrib* pAttrib);

    // ȡ�ڵ������ǵ�����(</Root);
    char*                           g_get_node_end_name(char*& pData, size_t& dwLen);

    bool                            g_is_blank( char* pCh );

    void                            g_skip_blank(char*&pData, size_t& dwLen);
    bool                            g_skip_comment( char*& pData, size_t& dwLen );


    // �ƶ�ָ��;
    void                            g_add_pointer( char*& pData, size_t& dwLen, size_t size = 1 );

}

