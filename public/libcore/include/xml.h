#pragma once
#include "xml_alloc.h"

namespace XML
{
    class CXmlAlloc;

    class CXmlFile
    {
    public:
        CXmlFile();
        ~CXmlFile();

    public:
        bool                Open(const char* szName);
        void                Close();

        inline char*        GetDataPtr(){return m_pBuffer;}
        inline size_t       GetSize(){return m_dwSize;}

    private:
        char*               m_pBuffer;
        size_t              m_dwSize;
    };


    // ����һ��char*�ַ����ı�;
    class CXmlString
    {
    public:
        CXmlString();
        ~CXmlString();

    public:
        void            SetValue(const char* pValue, size_t _size);
        inline char*    GetValue(){ return m_pBuffer; }

    private:
        char*           m_pBuffer;
        size_t          m_size;
    };


    // xml�ļ��е�����;
    class CXmlAttrib
    {
    public:
        CXmlAttrib(){}
       ~CXmlAttrib(){}

    public:
        void        SetName(const char* name, size_t len = 0) { attrName.SetValue(name, len); }
        void        SetProp(const char* prop, size_t len = 0) { attrProp.SetValue(prop, len); }

        char*       GetName() { return attrName.GetValue(); }
        char*       GetProp() { return attrProp.GetValue(); }

    private:
        CXmlString  attrName;
        CXmlString  attrProp;
    };


    class CXmlHead
    {
    public:
        CXmlHead();
        ~CXmlHead();

    public:
        bool                        Parse(char*& pData, size_t& size);
        void                        Close();

    private:
        std::list<CXmlAttrib*>      m_lstAttrib;
    };


    // xml �ļ��еĽڵ�;
    // �߼���value �� m_lstSonNodeֻ��һ������;
    class CXmlElement : public CXmlAttrib
    {
    public:
        CXmlElement() { m_pParentElement = nullptr; }
        ~CXmlElement(){ Close(); }

    public:
        inline CXmlElement*         GetParent() { return m_pParentElement; }
        inline void                 SetParent(CXmlElement* element ) { m_pParentElement = element; }

        bool                        Parse(char*& pData, size_t& dwLen );
        void                        Close();

        const std::list<CXmlAttrib*>&     GetAttribList()  { return m_lstAttrib; }
        const std::list<CXmlElement*>&    GetElementList() { return m_lstChildElement; }

        // ����������ȡ��ֵ;
        char*                       GetAttribValue(const char* pAttrName);
        void                        GetAttribValue(const char* pAttrName, int& nValue);

        // ���ݽڵ������ӽڵ�;
        CXmlElement*                GetElement(const char* pName);

    private:

        // �жϽڵ�Ľ�������Ƿ���ȷ;
        bool                        _verify_node_end_tag(char*& pData, size_t& dwLen);

        // ����������Ƿ�Ϸ�;
        bool                        _verify_attrib_name(char* pName, size_t dwLen);

        // �������ֵ�Ƿ�Ϸ�;
        bool                        _verify_attrib_value(char* pValue, size_t dwLen);

    private:
        CXmlElement*                m_pParentElement;    // ���ڵ�;
        std::list<CXmlElement*>     m_lstChildElement;   // �ӽڵ�;
        std::list<CXmlAttrib*>      m_lstAttrib;
    };


    class CXml
    {
    public:
        CXml();
        ~CXml();

    public:
        // ֱ�ӽ���xml�ļ�;
        bool                        ParseFromFile(const char* szName);

        // �����ڴ�xml����;
        bool                        ParseFromStream(char* szName, size_t dwLen );

        inline CXmlHead*            GetHead() { return &m_Head; }
        inline CXmlElement*         GetRoot() { return &m_Root; }

        void                        Close();

    private:
        bool                        _skip_utf8_head(char*& pData, size_t& dwLen);

    private:
        CXmlHead                    m_Head;    // ͷ�ڵ�; 
        CXmlElement                 m_Root;    // ���ڵ�;
        CXmlAlloc                   m_memAlloc;
    };

}
